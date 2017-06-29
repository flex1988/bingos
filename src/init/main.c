#include <stdint.h>

#include "multiboot.h"
#include "print/printk.h"
#include "mm/page.h"

static struct multiboot_info *_mbi;

void kmain(unsigned long addr, unsigned long init) {
    vga_init();
    printk("vga mode init...\n");

    _mbi = (struct multiboot_info *)addr;
    printk("mbi(0x%x) mods count(%d) %d", _mbi, _mbi->mods_count);

    page_init(_mbi);
    printk("page init...");

    
    while (1)
        ;
}
