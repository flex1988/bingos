#include <types.h>
#include "drivers/pci.h"
#include "kernel/mmu.h"
#include "kernel/process.h"
#include "module.h"

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

#define E1000_REG_CTRL 0x0000
#define E1000_REG_STATUS 0x0008
#define E1000_REG_EEPROM 0x0014
#define E1000_REG_CTRL_EXT 0x0018

struct rx_desc {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint16_t checknum;
    volatile uint8_t status;
    volatile uint8_t errors;
    volatile uint16_t special;
} __attribute__((packed));

struct tx_desc {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint8_t cso;
    volatile uint8_t cmd;
    volatile uint8_t status;
    volatile uint8_t css;
    volatile uint16_t special;
} __attribute__((packed));

static uint8_t *rx_virt[E1000_NUM_RX_DESC];
static uint8_t *tx_virt[E1000_NUM_TX_DESC];

extern page_dir_t *_kernel_pd;

static uint32_t e1000_device_pci = 0x00000000;
static uint32_t mem_base = 0;
static int has_eeprom = 0;
static uint8_t mac[6];
static list_t *net_queue = NULL;
static list_t *rx_wait = NULL;

static uint32_t mmio_read32(uint32_t addr) {
    return *((volatile uint32_t *)(addr));
}

static void mmio_write32(uint32_t addr, uint32_t val) {
    (*((volatile uint32_t *)(addr))) = val;
}

static void write_command(uint16_t addr, uint32_t val) {
    mmio_write32(mem_base + addr, val);
}

static uint32_t read_command(uint16_t addr) {
    return mmio_read32(mem_base + addr);
}

static int eeprom_detect(void) {
    write_command(E1000_REG_EEPROM, 1);

    for (int i = 0; i < 100000 && !has_eeprom; i++) {
        uint32_t val = read_command(E1000_REG_EEPROM);
        if (val & 0x10) {
            has_eeprom = 1;
            printk("E1000: eeprom detected");
        }
    }

    return 0;
}

static uint16_t eeprom_read(uint8_t addr) {
    uint32_t temp = 0;
    write_command(E1000_REG_EEPROM, 1 | ((uint32_t)(addr)) << 8);
    while (!((temp = read_command(E1000_REG_EEPROM)) && (1 << 4)))
        ;
    return (uint16_t)((temp >> 16) & 0xffff);
}

static void read_mac(void) {
    if (has_eeprom) {
        uint32_t t;
        t = eeprom_read(0);
        mac[0] = t & 0xff;
        mac[1] = t >> 8;
        t = eeprom_read(1);
        mac[2] = t & 0xff;
        mac[3] = t >> 8;
        t = eeprom_read(2);
        mac[4] = t & 0xff;
        mac[5] = t >> 8;
    } else {
        uint8_t *mac_addr = (uint8_t *)(mem_base + 0x5400);
        for (int i = 0; i < 6; i++) {
            mac[i] = mac_addr[i];
        }
    }

    printk("E1000: device mac %2x:%2x:%2x:%2x:%2x:%2x", mac[0], mac[1], mac[2],
           mac[3], mac[4], mac[5]);
}

static void find_e1000(uint32_t device, uint16_t vendorid, uint16_t deviceid,
                       void *extra) {
    if ((vendorid == 0x8086) &&
        (deviceid == 0x100e || deviceid == 0x1004 || deviceid == 0x100f)) {
        *((uint32_t *)extra) = device;
    }
}

static void e1000_init(char *name, void *data) {
    uint16_t command_reg = pci_read_field(e1000_device_pci, PCI_COMMAND, 2);
    command_reg |= (1 << 2);
    command_reg |= (1 << 0);
    pci_write_field(e1000_device_pci, PCI_COMMAND, 2, command_reg);

    eeprom_detect();
    read_mac();

    // initialize
    write_command(E1000_REG_CTRL, (1 << 26));

    process_sleep_until(CP, 0, 10);

    uint32_t status = read_command(E1000_REG_CTRL);
    status |= (1 << 5);
    status |= (1 << 6);
    status &= ~(1 << 3);
    status &= ~(1 << 31);
    status &= ~(1 << 7);
    write_command(E1000_REG_CTRL, status);

    write_command(0x0028, 0);
    write_command(0x002c, 0);
    write_command(0x0030, 0);
    write_command(0x0170, 0);

    status = read_command(E1000_REG_CTRL);
    status &= ~(1 << 30);
    write_command(E1000_REG_CTRL, status);

    process_sleep_until(CP, 0, 10);

    net_queue = list_create();
    rx_wait = list_create();

    process_exit(0);
}

int init(void) {
    printk("Loading e1000 module...");
    pci_scan(&find_e1000, -1, &e1000_device_pci);

    if (!e1000_device_pci) {
        printk("Not find e1000 device :( ...");
    }

    mem_base = pci_read_field(e1000_device_pci, PCI_BAR0, 4) & 0xfffffff0;
    printk("mem_base 0x%x", mem_base);

    page_t *page;
    for (size_t x = 0; x < 0x10000; x += 0x1000) {
        uint32_t addr = (mem_base & 0xfffff000) + x;
        page = get_page(addr, 1, _kernel_pd);
        page_identical_map(page, 1, 1, addr);
    }

    /*process_spawn_tasklet(e1000_init, "[e1000]", NULL);*/

    return 0;
}

int deinit(void) { return 0; }

MODULE_DEFINE(e1000, init, deinit);
