#include "fs/fs.h"
#include "kernel/kheap.h"

int do_open(const char *filename, int flags, int mode) {
    vfs_node_t *node = vfs_lookup(filename, flags);
    printk("open 0x%x",node);
    return 0;
}

int sys_open(const char *filename, int flags, int mode) {
    char *tmp;
    int error;

    error = do_open(filename, flags, mode);

    return error;
}
