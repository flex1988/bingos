#include <types.h>

#include "multiboot.h"
#include "print/printk.h"
#include "mm/frame.h"
#include "vga/vga.h"

static struct multiboot_info *_mbi;

void kmain(unsigned long addr) {
    vga_init();

    printk("vga mode init...");

    printk("multiboot_info start at 0x%x",addr);

    _mbi = (struct multiboot_info *)addr;

    frame_init(_mbi);

    uint32_t a = 0xfffff000;
    ptr_t b = get_physaddr(a);
    printk("b 0x%x",b);
    
    printk("page init...");

    while (1)
        ;
}