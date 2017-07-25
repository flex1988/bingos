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
#include "vga/vga.h"

extern ptr_t _placement_addr;
extern vfs_node_t *vfs_root;

void kmain(multiboot_info_t *boot_info) {
    vga_init();
    clear_screen();
    printk("vga mode init...");

    init_descriptor_tables();
    printk("init_descriptor_tables init...");

    asm volatile("sti");

    ASSERT(boot_info->mods_count > 0);

    uint32_t initrd = *((uint32_t *)boot_info->mods_addr);
    uint32_t initrd_end = *(uint32_t *)(boot_info->mods_addr + 4);
    // timer_init(50);
    _placement_addr = initrd_end;
    printk("initrd end: 0x%x", initrd_end);

    frame_init(boot_info);

    mmu_init();

    vfs_root = initrd_init(initrd);
    printk("initrd init...");

    /*int i = 0;*/
    /*dirent_t *node = 0;*/
    /*while ((node = vfs_readdir(vfs_root, i)) != 0) {*/
        /*printk(node->name);*/
        /*vfs_node_t *fsnode = vfs_finddir(vfs_root, node->name);*/

        /*if ((fsnode->flags & 0x7) == VFS_DIRECTORY)*/
            /*printk("\n\t(directory)\n");*/
        /*else {*/
            /*printk("\n\t contents: \"");*/
            /*char buf[256];*/
            /*uint32_t sz = vfs_read(fsnode, 0, 256, buf);*/
            /*printk(buf);*/
        /*}*/
        /*i++;*/
    /*}*/

    uint32_t b = kmalloc(8);
    printk("kmalloc 0x%x", b);
    kfree(b);
    uint32_t *ptr = (uint32_t *)0xA0000000;
    /*uint32_t do_page_fault = *ptr;*/

    while (1)
        ;
}
