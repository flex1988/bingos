#include <types.h>

#include "multiboot.h"
#include "print/printk.h"
#include "mm/frame.h"
#include "mm/mmu.h"
#include "vga/vga.h"
#include "hal/descriptor.h"

void kmain(unsigned long addr) {
    vga_init();
    clear_screen();
    printk("vga mode init...");

    init_descriptor_tables();
    printk("init_descriptor_tables init...");

    asm volatile("int $0x3");
    /*asm volatile("int $0x4");*/


    /*frame_init((struct multiboot_info *)addr);*/

    /*mmu_init();*/

    while (1)
        ;
}
