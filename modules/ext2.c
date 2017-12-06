#include "fs/ext2.h"
#include "fs/fs.h"
#include "kernel.h"
#include "module.h"

vfs_node_t *ext2_mount(char *device, char *mount_path) {
    char *arg = kmalloc(strlen(device) + 1);
    memcpy(arg, device, strlen(device) + 1);

    char *argv[10];
    ;
}

int ext2_init(void) {
    printk("Loading ext2 module...");

    return 0;
}

int ext2_deinit(void) { return 0; }

MODULE_DEFINE(ext2, ext2_init, ext2_deinit);
