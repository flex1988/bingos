#include <stdint.h>

#include "multiboot.h"
#include "print/printk.h"
#include "mm/page.h"
#include "vga/vga.h"

struct multiboot_info *_mbi;

void kmain(unsigned long addr,unsigned long magic) {
    vga_init();

    printk("vga mode init...");

    printk("multiboot_info start at 0x%x",addr);

    _mbi = (struct multiboot_info *)addr;

    page_init(_mbi);
    printk("page init...");

    while (1)
        ;
}
