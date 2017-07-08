#include <types.h>

#include "multiboot.h"
#include "print/printk.h"
#include "mm/frame.h"
#include "mm/mmu.h"
#include "vga/vga.h"

static struct multiboot_info *_mbi;

void kmain(unsigned long addr) {
    vga_init();
    clear_screen();
    printk("vga mode init...");

    printk("multiboot_info start at 0x%x",addr);
    _mbi = (struct multiboot_info *)addr;

    frame_init(_mbi);

    mmu_init();

    while (1)
        ;
}
