#include <types.h>

#include "fs/fs.h"
#include "fs/initrd.h"
#include "hal/descriptor.h"
#include "hal/timer.h"
#include "kernel.h"
#include "mm/frame.h"
#include "mm/mmu.h"
#include "multiboot.h"
#include "print/printk.h"
#include "proc/task.h"
#include "vga/vga.h"

extern ptr_t _placement_addr;
extern vfs_node_t *vfs_root;

extern uint32_t _ip;

uint32_t _initial_esp;

void kmain(multiboot_info_t *boot_info, uint32_t initial_stack) {
    _initial_esp = initial_stack;

    vga_init();
    clear_screen();
    printk("vga mode init...");

    printk("initial esp 0x%x", _initial_esp);

    init_descriptor_tables();
    printk("init_descriptor_tables init...");

    asm volatile("sti");

    ASSERT(boot_info->mods_count > 0);

    uint32_t initrd = *((uint32_t *)boot_info->mods_addr);
    uint32_t initrd_end = *(uint32_t *)(boot_info->mods_addr + 4);

    _placement_addr = initrd_end;
    printk("initrd end: 0x%x", initrd_end);

    frame_init(boot_info);

    mmu_init();

    task_init();

    timer_init(50);

    vfs_root = initrd_init(initrd);
    printk("initrd init...");

    int ret = fork();
    printk("ret eip 0x%x", read_eip());

    if (ret == 0) {
        while (1) printk("AAA");
    } else {
        while (1) printk("BBB");
    }

    while (1)
        ;
}
