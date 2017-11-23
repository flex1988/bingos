#include <types.h>

#include "fs/fs.h"
#include "fs/initrd.h"
#include "hal/descriptor.h"
#include "hal/timer.h"
#include "kernel.h"
#include "kernel/console.h"
#include "kernel/frame.h"
#include "kernel/mmu.h"
#include "kernel/printk.h"
#include "kernel/process.h"
#include "kernel/vesa.h"
#include "kernel/vga.h"
#include "multiboot.h"

extern ptr_t _placement_addr;
extern vfs_node_t *vfs_root;

extern uint32_t _ip;

uint32_t _initial_esp;

static void init(void) {}

extern process_t *_current_process;

extern console_t console;

static void message() {
    printk(
        "       _                _\n"
        " _ __ | |__   ___ _ __ (_)_  __\n"
        "| '_ \\| '_ \\ / _ \\ '_ \\| \\ \\/ /\n"
        "| |_) | | | |  __/ | | | |>  <\n"
        "| .__/|_| |_|\\___|_| |_|_/_/\\_\\\n"
        "|_|\n");
    printk("This is a mini x86 kernel");
    printk("Author: github.com/flex1988 <gao_chenfei@163.com> beijing China");
    printk("On 2017.10.20\n");
}

void kmain(multiboot_info_t *boot_info, uint32_t initial_stack) {
    _initial_esp = initial_stack;

    tty_init(boot_info);

    console.clear();

    init_descriptor_tables();

    IRQ_ON;

    timer_init(50);

    ASSERT(boot_info->mods_count > 0);

    uint32_t initrd = *((uint32_t *)boot_info->mods_addr);
    uint32_t initrd_end = *(uint32_t *)(boot_info->mods_addr + 4);

    _placement_addr = initrd_end;
    printk("initrd end: 0x%x", initrd_end);

    frame_init(boot_info);

    mmu_init();

    process_init();

    vfs_root = initrd_init(initrd);
    printk("initrd init...");

    syscalls_init();
    printk("syscalls init...");

    kbd_init();

    message();

    sys_exec("/init", 0, NULL);

    while (1)
        ;
}
