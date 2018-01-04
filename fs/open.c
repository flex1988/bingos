#include "fs/fs.h"
#include "kernel/kheap.h"
#include "kernel/process.h"

#include <errno.h>

int do_open(const char *filename, int flags, int mode) {
    int fd;

    for (fd = 0; fd < NR_OPEN; fd++) {
        if (!CP->fds->entries[fd])
            break;
    }

    if (fd >= NR_OPEN)
        return -EMFILE;

    vfs_node_t *node = vfs_lookup(filename, flags);
    vfs_open(node, 0);

    CP->fds->entries[fd] = node;
    CP->fds->length++;

    return fd;
}

int sys_open(const char *filename, int flags, int mode) {
    char *tmp;
    int error;

    error = do_open(filename, flags, mode);

    return error;
}

int sys_close(int fd) {
    if (fd > NR_OPEN)
        return -EBADF;

    vfs_node_t *n;

    if (!(n = CP->fds->entries[fd]))
        return -EBADF;
    
    CP->fds->entries[fd] = NULL;
    CP->fds->length--;

    return vfs_close(n);
}
