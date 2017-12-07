#include "fs/ext2.h"
#include "fs/fs.h"
#include "kernel.h"
#include "module.h"

static vfs_node_t *ext2_mount(char *device, char *mount_path) {
    vfs_node_t *dev = vfs_lookup(device, 0);

    printk("ext2 mount 0x%x", dev);
    return NULL;
}

int ext2_init(void) {
    printk("Loading ext2 module...");
    vfs_register("ext2", ext2_mount);
    return 0;
}

int ext2_deinit(void) { return 0; }

MODULE_DEFINE(ext2, ext2_init, ext2_deinit);
