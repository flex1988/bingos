#include "drivers/ata.h"
#include "drivers/pci.h"
#include "fs/fs.h"
#include "hal/common.h"
#include "hal/isr.h"
#include "kernel.h"
#include "kernel/kheap.h"
#include "lib/list.h"
#include "module.h"

#include <stdio.h>

static char ata_drive_char = 'a';
static int cdrom_number = 0;
static uint32_t ata_pci = 0x00000000;
static list_t *atapi_waiter = NULL;
static int atapi_in_progress = 0;

typedef union {
    uint8_t command_bytes[12];
    uint16_t command_words[6];
} atapi_command_t;

static void find_ata_pci(uint32_t device, uint16_t vendorid, uint16_t deviceid, void *extra) {
    if ((vendorid == 0x8086) && (deviceid == 0x7010 || deviceid == 0x7111)) {
        *((uint32_t *)extra) = device;
    }
}

typedef struct {
    uint32_t offset;
    uint16_t bytes;
    uint16_t last;
} prdt_t;

typedef struct {
    int io_base;
    int control;
    int slave;
    int is_atapi;
    ata_identify_t identity;
    prdt_t *dma_prdt;
    uint32_t dma_prdt_phys;
    uint8_t *dma_start;
    uint32_t dma_start_phys;
    uint32_t bar4;
    uint32_t atapi_lba;
    uint32_t atapi_sector_size;
} ata_device_t;

static ata_device_t ata_primary_master = {.io_base = 0x1f0, .control = 0x3f6, .slave = 0};
static ata_device_t ata_primary_slave = {.io_base = 0x1f0, .control = 0x3f6, .slave = 1};
static ata_device_t ata_secondary_master = {.io_base = 0x170, .control = 0x376, .slave = 0};
static ata_device_t ata_secondary_slave = {.io_base = 0x170, .control = 0x376, .slave = 1};

#define ATA_SECTOR_SIZE 512

static void ata_device_read_sector(ata_device_t *dev, uint32_t lba, uint8_t *buf);
static void ata_device_read_sector_atapi(ata_device_t *dev, uint32_t lba, uint8_t *buf);
static void ata_device_write_sector_retry(ata_device_t *dev, uint32_t lba, uint8_t *buf);
static uint32_t read_ata(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
static uint32_t write_ata(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
static void open_ata(vfs_node_t *node, uint32_t flags);
static void close_ata(vfs_node_t *node);

static uint64_t ata_max_offset(ata_device_t *dev) {
    uint64_t sectors = dev->identity.sectors_48;
    if (!sectors) {
        sectors = dev->identity.sectors_28;
    }

    return sectors * ATA_SECTOR_SIZE;
}

static uint64_t atapi_max_offset(ata_device_t *dev) {
    uint64_t max_sector = dev->atapi_lba;

    if (!max_sector)
        return 0;

    return (max_sector + 1) * dev->atapi_sector_size;
}

static uint32_t read_ata(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    ata_device_t *dev = (ata_device_t *)node->device;

    uint32_t start_block = offset / ATA_SECTOR_SIZE;
    uint32_t end_block = (offset + size - 1) / ATA_SECTOR_SIZE;

    uint32_t x_offset = 0;

    if (offset % ATA_SECTOR_SIZE) {
        uint32_t prefix_size = (ATA_SECTOR_SIZE - (offset % ATA_SECTOR_SIZE));
        char *tmp = kmalloc(ATA_SECTOR_SIZE);

        ide_read_sector(dev->io_base, dev->slave, start_block, (uint8_t *)tmp);
        memcpy(buffer, (void *)((uint32_t)tmp + (offset % ATA_SECTOR_SIZE)), prefix_size);

        kfree(tmp);

        x_offset += prefix_size;
        start_block++;
    }

    if ((offset + size) % ATA_SECTOR_SIZE && start_block <= end_block) {
        uint32_t postfix_size = (offset + size) % ATA_SECTOR_SIZE;
        char *tmp = kmalloc(ATA_SECTOR_SIZE);
        ide_read_sector(dev->io_base, dev->slave, end_block, (uint8_t *)tmp);

        memcpy((void *)((uint32_t)buffer + size - postfix_size), tmp, postfix_size);

        kfree(tmp);

        end_block--;
    }

    while (start_block <= end_block) {
        ide_read_sector(dev->io_base, dev->slave, start_block, (uint8_t *)((uint32_t)buffer + x_offset));
        x_offset += ATA_SECTOR_SIZE;
        start_block++;
    }

    return size;
}

static uint32_t read_atapi(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    ata_device_t *dev = (ata_device_t *)node->device;

    uint32_t start_block = offset / dev->atapi_sector_size;
    uint32_t end_block = (offset + size - 1) / dev->atapi_sector_size;

    uint32_t x_offset = 0;

    if (offset > atapi_max_offset(dev)) {
        return 0;
    }

    if (offset + size > atapi_max_offset(dev)) {
        uint32_t i = atapi_max_offset(dev) - offset;
        size = i;
    }

    if (offset % dev->atapi_sector_size) {
        uint32_t prefix_size = (dev->atapi_sector_size - (offset % dev->atapi_sector_size));
        char *tmp = kmalloc(dev->atapi_sector_size);
        ata_device_read_sector_atapi(dev, start_block, (uint8_t *)tmp);

        memcpy(buffer, (void *)((uint32_t)tmp + (offset % dev->atapi_sector_size)), prefix_size);

        kfree(tmp);

        x_offset += prefix_size;
        start_block++;
    }

    if ((offset + size) % dev->atapi_sector_size && start_block <= end_block) {
        unsigned int postfix_size = (offset + size) % dev->atapi_sector_size;
        char *tmp = kmalloc(dev->atapi_sector_size);
        ata_device_read_sector_atapi(dev, end_block, (uint8_t *)tmp);

        memcpy((void *)((uint32_t)buffer + size - postfix_size), tmp, postfix_size);

        kfree(tmp);

        end_block--;
    }

    while (start_block <= end_block) {
        ata_device_read_sector_atapi(dev, start_block, (uint8_t *)((uint32_t)buffer + x_offset));
        x_offset += dev->atapi_sector_size;
        start_block++;
    }

    return size;
}

static uint32_t write_ata(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    ata_device_t *dev = (ata_device_t *)node->device;

    uint32_t start_block = offset / ATA_SECTOR_SIZE;
    uint32_t end_block = (offset + size - 1) / ATA_SECTOR_SIZE;

    uint32_t x_offset = 0;

    if (offset > ata_max_offset(dev)) {
        return 0;
    }

    if (offset + size > ata_max_offset(dev)) {
        uint32_t i = ata_max_offset(dev) - offset;
        size = i;
    }

    if (offset % ATA_SECTOR_SIZE) {
        uint32_t prefix_size = (ATA_SECTOR_SIZE - (offset % ATA_SECTOR_SIZE));

        char *tmp = kmalloc(ATA_SECTOR_SIZE);
        ata_device_read_sector(dev, start_block, (uint8_t *)tmp);

        printk("Writing first block");

        memcpy((void *)((uint32_t)tmp + (offset % ATA_SECTOR_SIZE)), buffer, prefix_size);
        ata_device_write_sector_retry(dev, start_block, (uint8_t *)tmp);

        kfree(tmp);
        x_offset += prefix_size;
        start_block++;
    }

    if ((offset + size) % ATA_SECTOR_SIZE && start_block <= end_block) {
        uint32_t postfix_size = (offset + size) % ATA_SECTOR_SIZE;

        char *tmp = kmalloc(ATA_SECTOR_SIZE);
        ata_device_read_sector(dev, end_block, (uint8_t *)tmp);

        printk("Writing last block");

        memcpy(tmp, (void *)((uint32_t)buffer + size - postfix_size), postfix_size);

        ata_device_write_sector_retry(dev, end_block, (uint8_t *)tmp);

        kfree(tmp);
        end_block--;
    }

    while (start_block <= end_block) {
        ata_device_write_sector_retry(dev, start_block, (uint8_t *)((uint32_t)buffer + x_offset));
        x_offset += ATA_SECTOR_SIZE;
        start_block++;
    }

    return size;
}

static void open_ata(vfs_node_t *node, uint32_t flags) { return; }

static void close_ata(vfs_node_t *node) { return; }

static vfs_node_t *atapi_device_create(ata_device_t *device) {
    vfs_node_t *node = kmalloc(sizeof(vfs_node_t));
    memset(node, 0x0, sizeof(vfs_node_t));
    node->inode = 0;
    sprintf(node->name, "cdrom%d", cdrom_number);
    node->device = device;
    node->uid = 0;
    node->gid = 0;
    node->mask = 0660;
    node->length = atapi_max_offset(device);
    node->flags = VFS_BLOCKDEVICE;
    node->read = read_atapi;
    node->write = NULL;
    node->open = open_ata;
    node->close = close_ata;
    node->readdir = NULL;
    node->finddir = NULL;
    node->ioctl = NULL;
    return node;
}

static vfs_node_t *ata_device_create(ata_device_t *device) {
    vfs_node_t *node = kmalloc(sizeof(vfs_node_t));
    memset(node, 0x00, sizeof(vfs_node_t));
    node->inode = 0;
    sprintf(node->name, "atadev%d", ata_drive_char - 'a');
    node->device = device;
    node->uid = 0;
    node->gid = 0;
    node->mask = 0660;
    node->length = ata_max_offset(device); /* TODO */
    node->flags = VFS_BLOCKDEVICE;
    node->read = read_ata;
    node->write = write_ata;
    node->open = open_ata;
    node->close = close_ata;
    node->readdir = NULL;
    node->finddir = NULL;
    node->ioctl = NULL; /* TODO, identify, etc? */
    return node;
}

static void ata_io_wait(ata_device_t *dev) {
    inb(dev->io_base + ATA_REG_ALTSTATUS);
    inb(dev->io_base + ATA_REG_ALTSTATUS);
    inb(dev->io_base + ATA_REG_ALTSTATUS);
    inb(dev->io_base + ATA_REG_ALTSTATUS);
}

static int ata_status_wait(ata_device_t *dev, int timeout) {
    int status;
    if (timeout > 0) {
        int i = 0;
        while ((status = inb(dev->io_base + ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout)) i++;
    } else {
        while ((status = inb(dev->io_base + ATA_REG_STATUS)) & ATA_SR_BSY)
            ;
    }
    return status;
}

static int ata_wait(ata_device_t *dev, int advanced) {
    uint8_t status = 0;

    ata_io_wait(dev);

    status = ata_status_wait(dev, -1);

    if (advanced) {
        status = inb(dev->io_base + ATA_REG_STATUS);
        if (status & ATA_SR_ERR)
            return 1;
        if (status & ATA_SR_DF)
            return 1;
        if (!(status & ATA_SR_DRQ))
            return 1;
    }

    return 0;
}

static void ata_soft_reset(ata_device_t *dev) {
    outb(dev->control, 0x04);
    ata_io_wait(dev);
    outb(dev->control, 0x00);
}

static int ata_device_detect(ata_device_t *dev) {
    uint8_t cl = inb(dev->io_base + ATA_REG_LBA1); /* CYL_LO */
    uint8_t ch = inb(dev->io_base + ATA_REG_LBA2); /* CYL_HI */

    printk("Device detected: 0x%2x 0x%2x", cl, ch);
    if (cl == 0xFF && ch == 0xFF) {
        /* Nothing here */
        return 0;
    }
    if ((cl == 0x00 && ch == 0x00) || (cl == 0x3C && ch == 0xC3)) {
        /* Parallel ATA device, or emulated SATA */

        char devname[64];
        sprintf((char *)&devname, "/dev/hd%c", ata_drive_char);
        vfs_node_t *node = ata_device_create(dev);
        vfs_mount(devname, node);
        ata_drive_char++;

        return 1;
    }

    /* TODO: ATAPI, SATA, SATAPI */
    return 0;
}

static void ata_device_read_sector(ata_device_t *dev, uint32_t lba, uint8_t *buf) {
    uint16_t bus = dev->io_base;
    uint8_t slave = dev->slave;

    if (dev->is_atapi)
        return;

    /*spin_lock(ata_lock);*/

    ata_wait(dev, 0);

    /* Stop */
    outb(dev->bar4, 0x00);

    /* Set the PRDT */
    outl(dev->bar4 + 0x04, dev->dma_prdt_phys);

    /* Enable error, irq status */
    outb(dev->bar4 + 0x2, inb(dev->bar4 + 0x02) | 0x04 | 0x02);

    /* set read */
    outb(dev->bar4, 0x08);

    IRQ_ON;

    while (1) {
        uint8_t status = inb(dev->io_base + ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY))
            break;
    }

    outb(bus + ATA_REG_CONTROL, 0x00);
    outb(bus + ATA_REG_HDDEVSEL, 0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
    ata_io_wait(dev);
    outb(bus + ATA_REG_FEATURES, 0x00);
    outb(bus + ATA_REG_SECCOUNT0, 1);
    outb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >> 0);
    outb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >> 8);
    outb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
    while (1) {
        uint8_t status = inb(dev->io_base + ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRDY))
            break;
    }
    outb(bus + ATA_REG_COMMAND, ATA_CMD_READ_DMA);

    ata_io_wait(dev);

    outb(dev->bar4, 0x08 | 0x01);

    while (1) {
        int status = inb(dev->bar4 + 0x02);
        int dstatus = inb(dev->io_base + ATA_REG_STATUS);
        if (!(status & 0x04)) {
            continue;
        }
        if (!(dstatus & ATA_SR_BSY)) {
            break;
        }
    }
    IRQ_OFF;

    /* Copy from DMA buffer to output buffer. */
    memcpy(buf, dev->dma_start, 512);

    /* Inform device we are done. */
    outb(dev->bar4 + 0x2, inb(dev->bar4 + 0x02) | 0x04 | 0x02);

    /*spin_unlock(ata_lock);*/
}

static void ata_device_read_sector_atapi(ata_device_t *dev, uint32_t lba, uint8_t *buf) {
    if (!dev->is_atapi)
        return;

    uint16_t bus = dev->io_base;
    /*spin_lock(ata_lock);*/

    outb(dev->io_base + ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
    ata_io_wait(dev);

    outb(bus + ATA_REG_FEATURES, 0x00);
    outb(bus + ATA_REG_LBA1, dev->atapi_sector_size & 0xFF);
    outb(bus + ATA_REG_LBA2, dev->atapi_sector_size >> 8);
    outb(bus + ATA_REG_COMMAND, ATA_CMD_PACKET);

    /* poll */
    while (1) {
        uint8_t status = inb(dev->io_base + ATA_REG_STATUS);
        if ((status & ATA_SR_ERR))
            goto atapi_error_on_read_setup;
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
            break;
    }

    atapi_in_progress = 1;

    atapi_command_t command;
    command.command_bytes[0] = 0xA8;
    command.command_bytes[1] = 0;
    command.command_bytes[2] = (lba >> 0x18) & 0xFF;
    command.command_bytes[3] = (lba >> 0x10) & 0xFF;
    command.command_bytes[4] = (lba >> 0x08) & 0xFF;
    command.command_bytes[5] = (lba >> 0x00) & 0xFF;
    command.command_bytes[6] = 0;
    command.command_bytes[7] = 0;
    command.command_bytes[8] = 0; /* bit 0 = PMI (0, last sector) */
    command.command_bytes[9] = 1; /* control */
    command.command_bytes[10] = 0;
    command.command_bytes[11] = 0;

    for (int i = 0; i < 6; ++i) {
        outs(bus, command.command_words[i]);
    }

    /* Wait */
    /*sleep_on(atapi_waiter);*/

    atapi_in_progress = 0;

    while (1) {
        uint8_t status = inb(dev->io_base + ATA_REG_STATUS);
        if ((status & ATA_SR_ERR))
            goto atapi_error_on_read_setup;
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
            break;
    }

    uint16_t size_to_read = inb(bus + ATA_REG_LBA2) << 8;
    size_to_read = size_to_read | inb(bus + ATA_REG_LBA1);

    insm(bus, buf, size_to_read / 2);

    while (1) {
        uint8_t status = inb(dev->io_base + ATA_REG_STATUS);
        if ((status & ATA_SR_ERR))
            goto atapi_error_on_read_setup;
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRDY))
            break;
    }

atapi_error_on_read_setup:; /*spin_unlock(ata_lock);*/
}

static void ata_device_write_sector(ata_device_t *dev, uint32_t lba, uint8_t *buf) {
    uint16_t bus = dev->io_base;
    uint8_t slave = dev->slave;

    /*spin_lock(ata_lock);*/

    outb(bus + ATA_REG_CONTROL, 0x02);

    ata_wait(dev, 0);
    outb(bus + ATA_REG_HDDEVSEL, 0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
    ata_wait(dev, 0);

    outb(bus + ATA_REG_FEATURES, 0x00);
    outb(bus + ATA_REG_SECCOUNT0, 0x01);
    outb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >> 0);
    outb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >> 8);
    outb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
    outb(bus + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    ata_wait(dev, 0);
    int size = ATA_SECTOR_SIZE / 2;
    outsm(bus, buf, size);
    outb(bus + 0x07, ATA_CMD_CACHE_FLUSH);
    /*spin_unlock(ata_lock);*/
    ata_wait(dev, 0);
}

static int buffer_compare(uint32_t *ptr1, uint32_t *ptr2, size_t size) {
    size_t i = 0;
    while (i < size) {
        if (*ptr1 != *ptr2)
            return 1;
        ptr1++;
        ptr2++;
        i += sizeof(uint32_t);
    }
    return 0;
}

static void ata_device_write_sector_retry(ata_device_t *dev, uint32_t lba, uint8_t *buf) {
    uint8_t *read_buf = kmalloc(ATA_SECTOR_SIZE);
    do {
        ata_device_write_sector(dev, lba, buf);
        ata_device_read_sector(dev, lba, read_buf);
    } while (buffer_compare((uint32_t *)buf, (uint32_t *)read_buf, ATA_SECTOR_SIZE));

    kfree(read_buf);
}

int ata_init(void) {
    printk("Loading ata module...");

    pci_scan(&find_ata_pci, -1, &ata_pci);

    ata_device_detect(&ata_primary_master);
    ata_device_detect(&ata_primary_slave);

    return 0;
}

int ata_deinit(void) { return 0; }

MODULE_DEFINE(ata, ata_init, ata_deinit);
