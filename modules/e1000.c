#include <types.h>
#include "drivers/pci.h"
#include "module.h"

static uint32_t e1000_device_pci = 0x00000000;

static void find_e1000(uint32_t device, uint16_t vendorid, uint16_t deviceid, void *extra) {
    if ((vendorid == 0x8086) && (deviceid == 0x100e || deviceid == 0x1004 || deviceid == 0x100f)) {
        *((uint32_t *)extra) = device;
    }
}

int e1000_init(void) {
    printk("Loading e1000 module...");
    pci_scan(&find_e1000, -1, &e1000_device_pci);

    if(!e1000_device_pci) {
        printk("Not find e1000 device :( ...");
    }

    return 0;
}

int e1000_deinit(void) { return 0; }

MODULE_DEFINE(e1000, e1000_init, e1000_deinit);
