#include <types.h>

#include "hal/descriptor.h"
#include "hal/timer.h"
#include "mm/frame.h"
#include "mm/mmu.h"
#include "multiboot.h"
#include "print/printk.h"
#include "vga/vga.h"

void kmain(unsigned long addr) {
    vga_init();
    clear_screen();
    printk("vga mode init...");

    init_descriptor_tables();
    printk("init_descriptor_tables init...");

    asm volatile("sti");

    timer_init(50);

    frame_init((struct multiboot_info *)addr);

    mmu_init();

    uint32_t *ptr = (uint32_t *)0xA0000000;
    /*uint32_t do_page_fault = *ptr;*/

    while (1)
        ;
}
