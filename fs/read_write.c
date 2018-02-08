#include "fs/fs.h"
#include "kernel.h"
#include "kernel/process.h"

static int validate_fd(uint32_t fd) {
    if (fd >= CP->fds->length) {
        return 0;
    }

    if (!CP->fds->entries[fd]) {
        return 0;
    }

    return 1;
}

int sys_read(uint32_t fd, char *buf, uint32_t count) {
    if (!validate_fd(fd)) {
        printk("invalid fd");
        return -1;
    }

    vfs_node_t *node = CP->fds->entries[fd];

    int nread = vfs_read(node, node->offset, count, buf);

    node->offset += nread;

    return nread;
}

int sys_readdir(int fd, int index, dirent_t *dir) {
    if (!validate_fd(fd)) {
        printk("invalid fd");
        return -1;
    }

    dirent_t *entry = vfs_readdir(CP->fds->entries[fd], index);

    if (entry) {
        memcpy(dir, entry, sizeof(dirent_t));
        return 0;
    } else {
        return 1;
    }
}

int sys_write(int fd, const char *buf, uint32_t size) {
    if (!validate_fd(fd)) {
        printk("invalid fd");
        return -1;
    }

    vfs_node_t *node = CP->fds->entries[fd];

    return vfs_write(node, node->offset, size, buf);
}
