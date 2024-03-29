#include <types.h>

#include "fs/fs.h"
#include "fs/ext2.h"
#include "fs/initrd.h"
#include "fs/devfs.h"
#include "hal/descriptor.h"
#include "kernel.h"
#include "kernel/console.h"
#include "kernel/frame.h"
#include "kernel/malloc.h"
#include "kernel/mmu.h"
#include "kernel/printk.h"
#include "kernel/process.h"
#include "kernel/timer.h"
#include "kernel/vesa.h"
#include "kernel/vga.h"
#include "lib/tree.h"
#include "module.h"
#include "multiboot.h"

uint32_t _initial_esp;

static void message() {
    printk("\n\n");
    printk("          ______  _________ _        _______  _______  _______\n \ 
        (  ___ \\ \\__   __/( (    /|(  ____ \\(  ___  )(  ____ \\\n\
         | (   ) )   ) (   |  \\  ( || (    \\\/| (   ) || (    \\\/\n\
         | (__/ /    | |   |   \\ | || |      | |   | || (_____ \n\
         |  __ (     | |   | (\\ \\) || | ____ | |   | |(_____  )\n\
         | (  \\ \\    | |   | | \\   || | \\_  )| |   | |      ) |\n\
         | )___) )___) (___| )  \\  || (___) || (___) |/\\____) |\n\
         |/ \\___/ \\_______/|/    )_)(_______)(_______)\\_______)\n\
    ");
    printk("\n\n");
    
    printk("This is a mini x86 kernel");
    printk("Author: github.com/flex1988 <gao_chenfei@163.com> 2017.10.20,beijing,China");
    printk("\n\n");
}

static void pci_print(uint32_t device, uint16_t vendor_id, uint16_t device_id, void* extra)
{
    printk("device %d vendor %d deviceid %d extra %p", device, vendor_id, device_id, extra);
}


void kmain(multiboot_info_t *boot_info, uint32_t initial_stack) {
    _initial_esp = initial_stack;

    tty_init(boot_info);
    init_descriptor_tables();
    frame_init(boot_info);
    mmu_init();
    process_init();
    timer_init();
    vfs_init();
    devfs_init();
    modules_init(boot_info);
    ext2_init();
    ramdisk_init(boot_info);
    vfs_mount_type("ext2", "/dev/atadev0", "/root");

    sock_init();
    syscalls_init();

    sys_exec("/bin/init", 0, NULL);
    context_switch(0);

    return 0;
}
