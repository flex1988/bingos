#include <stdint.h>

#include "multiboot.h"
#include "print/printk.h"
#include "mm/page.h"
#include "vga/vga.h"

void kmain() {
    vga_init();
    printk("vga mode init...\n");

    //printk("magic %x", _mbi->magic);

    //page_init(_mbi);
    printk("page init...");

    while (1)
        ;
}
