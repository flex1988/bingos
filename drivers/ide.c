#include "drivers/ata.h"

ide_channel_t ide_channels[2];
ide_device_t ide_devices[4];
uint8_t ide_buf[2048] = {0};
uint8_t ide_irq_invoked = 0;
uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void ide_detect();
uint8_t ide_read(uint8_t channel, uint8_t reg);
void ide_write(uint8_t channel, uint8_t reg, uint8_t data);

uint8_t ide_read(uint8_t channel, uint8_t reg) {
    uint8_t result;
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].nien);
    if (reg < 0x08)
        result = inb(ide_channels[channel].base + reg - 0x00);
    else if (reg < 0x0c)
        result = inb(ide_channels[channel].base + reg - 0x06);
    else if (reg < 0x0e)
        result = inb(ide_channels[channel].ctrl + reg - 0x0a);
    else if (reg < 0x16)
        result = inb(ide_channels[channel].bmide + reg - 0x0e);
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].nien);
    return result;
}

void ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].nien);
    if (reg < 0x08)
        outb(data, ide_channels[channel].base + reg - 0x00);
    else if (reg < 0x0c)
        outb(data, ide_channels[channel].base + reg - 0x06);
    else if (reg < 0x0e)
        outb(data, ide_channels[channel].ctrl + reg - 0x0a);
    else if (reg < 0x16)
        outb(data, ide_channels[channel].bmide + reg - 0x0e);
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].nien);
}

void ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t buffer, uint32_t quads) {
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].nien);
    __asm__ __volatile__("pushw %es; movw %ds, %ax; movw %ax, %es");
    if (reg < 0x08)
        insl(ide_channels[channel].base + reg - 0x00, buffer, quads);
    else if (reg < 0x0C)
        insl(ide_channels[channel].base + reg - 0x06, buffer, quads);
    else if (reg < 0x0E)
        insl(ide_channels[channel].ctrl + reg - 0x0A, buffer, quads);
    else if (reg < 0x16)
        insl(ide_channels[channel].bmide + reg - 0x0E, buffer, quads);
    __asm__ __volatile__("popw %es;");
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].nien);
}

uint8_t ide_polling(uint8_t channel, uint32_t advanced_check) {
    // (I) Delay 400 nanosecond for BSY to be set:
    // -------------------------------------------------
    ide_read(channel, ATA_REG_ALTSTATUS);  // Reading Alternate Status Port wastes 100ns.
    ide_read(channel, ATA_REG_ALTSTATUS);  // Reading Alternate Status Port wastes 100ns.
    ide_read(channel, ATA_REG_ALTSTATUS);  // Reading Alternate Status Port wastes 100ns.
    ide_read(channel, ATA_REG_ALTSTATUS);  // Reading Alternate Status Port wastes 100ns.

    // (II) Wait for BSY to be cleared:
    // -------------------------------------------------
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
        ;  // Wait for BSY to be zero.

    if (advanced_check) {
        uint8_t state = ide_read(channel, ATA_REG_STATUS);  // Read Status Register.

        // (III) Check For Errors:
        // -------------------------------------------------
        if (state & ATA_SR_ERR)
            return 2;  // Error.

        // (IV) Check If Device fault:
        // -------------------------------------------------
        if (state & ATA_SR_DF)
            return 1;  // Device Fault.

        // (V) Check DRQ:
        // -------------------------------------------------
        // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
        if (!(state & ATA_SR_DRQ))
            return 3;  // DRQ should be set
    }

    return 0;  // No Error.
}

uint8_t ide_print_error(uint32_t drive, uint8_t err) {
    if (err == 0)
        return err;

    printk(" IDE:");
    if (err == 1) {
        printk("- Device Fault\n     ");
        err = 19;
    } else if (err == 2) {
        uint8_t st = ide_read(ide_devices[drive].channel, ATA_REG_ERROR);
        if (st & ATA_ER_AMNF) {
            printk("- No Address Mark Found\n     ");
            err = 7;
        }
        if (st & ATA_ER_TK0NF) {
            printk("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_ABRT) {
            printk("- Command Aborted\n     ");
            err = 20;
        }
        if (st & ATA_ER_MCR) {
            printk("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_IDNF) {
            printk("- ID mark not Found\n     ");
            err = 21;
        }
        if (st & ATA_ER_MC) {
            printk("- No Media or Media Error\n     ");
            err = 3;
        }
        if (st & ATA_ER_UNC) {
            printk("- Uncorrectable Data Error\n     ");
            err = 22;
        }
        if (st & ATA_ER_BBK) {
            printk("- Bad Sectors\n     ");
            err = 13;
        }
    } else if (err == 3) {
        printk("- Reads Nothing\n     ");
        err = 23;
    } else if (err == 4) {
        printk("- Write Protected\n     ");
        err = 8;
    }
    printk("- [%s %s] %s\n", (const char *[]){"Primary", "Secondary"}[ide_devices[drive].channel],
           (const char *[]){"Master", "Slave"}[ide_devices[drive].drive], ide_devices[drive].model);

    return err;
}

void ide_initialize(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4) {
    int i, j, k, count = 0;

    // 1- Detect I/O Ports which interface IDE Controller:
    ide_channels[ATA_PRIMARY].base = (BAR0 &= 0xFFFFFFFC) + 0x1F0 * (!BAR0);
    ide_channels[ATA_PRIMARY].ctrl = (BAR1 &= 0xFFFFFFFC) + 0x3F4 * (!BAR1);
    ide_channels[ATA_SECONDARY].base = (BAR2 &= 0xFFFFFFFC) + 0x170 * (!BAR2);
    ide_channels[ATA_SECONDARY].ctrl = (BAR3 &= 0xFFFFFFFC) + 0x374 * (!BAR3);
    ide_channels[ATA_PRIMARY].bmide = (BAR4 &= 0xFFFFFFFC) + 0;    // Bus Master IDE
    ide_channels[ATA_SECONDARY].bmide = (BAR4 &= 0xFFFFFFFC) + 8;  // Bus Master IDE

    // 2- Disable IRQs:
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

    // 3- Detect ATA-ATAPI Devices:
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++) {
            uint8_t err = 0, type = IDE_ATA, status;
            ide_devices[count].reserved = 0;  // Assuming that no drive here.

            // (I) Select Drive:
            ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));  // Select Drive.

            // (II) Send ATA Identify Command:
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            /*sleep(1);*/
            // (III) Polling:
            if (!(ide_read(i, ATA_REG_STATUS))) {
                continue;  // If Status = 0, No Device.
            }

            while (1) {
                status = ide_read(i, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR)) {
                    err = 1;
                    break;
                }  // If Err, Device is not ATA.
                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
                    break;  // Everything is right.
            }

            // (IV) Probe for ATAPI Devices:
            if (err) {
                uint8_t cl = ide_read(i, ATA_REG_LBA1);
                uint8_t ch = ide_read(i, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB)
                    type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96)
                    type = IDE_ATAPI;
                else
                    continue;  // Unknown Type (And always not be a device).

                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                /*sleep(2);*/
            }

            // (V) Read Identification Space of the Device:
            ide_read_buffer(i, ATA_REG_DATA, (uint32_t)ide_buf, 128);

            // (VI) Read Device Parameters:
            ide_devices[count].reserved = 1;
            ide_devices[count].type = type;
            ide_devices[count].channel = i;
            ide_devices[count].drive = j;
            ide_devices[count].signature = ((uint16_t *)(ide_buf + ATA_IDENT_DEVICETYPE))[0];
            ide_devices[count].capabilities = ((uint16_t *)(ide_buf + ATA_IDENT_CAPABILITIES))[0];
            ide_devices[count].commands_sets = ((uint32_t *)(ide_buf + ATA_IDENT_COMMANDSETS))[0];

            // (VII) Get Size:
            if (ide_devices[count].commands_sets & (1 << 26)) {
                // Device uses 48-Bit Addressing:
                ide_devices[count].size = ((uint32_t *)(ide_buf + ATA_IDENT_MAX_LBA_EXT))[0];
                // Note that Quafios is 32-Bit Operating System, So last 2 Words are ignored.
            } else {
                // Device uses CHS or 28-bit Addressing:
                ide_devices[count].size = ((uint32_t *)(ide_buf + ATA_IDENT_MAX_LBA))[0];
            }

            // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
            for (k = ATA_IDENT_MODEL; k < (ATA_IDENT_MODEL + 40); k += 2) {
                ide_devices[count].model[k - ATA_IDENT_MODEL] = ide_buf[k + 1];
                ide_devices[count].model[(k + 1) - ATA_IDENT_MODEL] = ide_buf[k];
            }
            ide_devices[count].model[40] = 0;  // Terminate String.

            count++;
        }

    // 4- Print Summary:
    for (i = 0; i < 4; i++)
        if (ide_devices[i].reserved == 1) {
            printk(" Found %s Drive %dGB - %s\n", (const char *[]){"ATA", "ATAPI"}[ide_devices[i].type], /* Type */
                   ide_devices[i].size / 1024 / 1024 / 2,                                                /* Size */
                   ide_devices[i].model);
        }
}

void ide_read_sector(uint16_t bus, uint8_t slave, uint32_t lba, uint8_t *buf) {
    outb(bus + ATA_REG_FEATURES, 0x0);
    outb(bus + ATA_REG_SECCOUNT0, 1);
    outb(buf + ATA_REG_HDDEVSEL, 0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);

    outb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >> 0);
    outb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >> 8);
    outb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
    outb(bus + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    uint8_t status = 0;
    while ((status = inb(bus + 0x07)) & 0x80)
        ;
    int size = 256;
    insm(bus, buf, size);
}

void ide_write_sector(uint16_t bus, uint8_t slave, uint32_t lba, uint8_t *buf) {
    outb(bus + ATA_REG_FEATURES, 0x00);
    outb(bus + ATA_REG_SECCOUNT0, 0x01);
    outb(bus + ATA_REG_HDDEVSEL, 0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);

    outb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >> 0);
    outb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >> 8);
    outb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
    outb(bus + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    uint8_t status = 0;
    while ((status = inb(bus + 0x07)) & 0x80)
        ;
    int size = 256;
    insm(bus, buf, size);
}
