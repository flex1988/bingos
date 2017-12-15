#include "fs/fs.h"
#include "kernel.h"
#include "kernel/process.h"

extern process_t *_current_process;

static int validate_fd(uint32_t fd) {
    if (fd >= _current_process->fds->length) {
        printk("xx %d", _current_process->fds->length);
        return 0;
    }

    if (!_current_process->fds->entries[fd]) {
        return 0;
    }

    return 1;
}

int sys_read(uint32_t fd, char *buf, uint32_t count) {
    if (!validate_fd(fd)) {
        printk("invalid fd");
        return -1;
    }

    vfs_node_t *node = _current_process->fds->entries[fd];

    int nread = vfs_read(node, node->offset, count, buf);

    node->offset += nread;

    return nread;
}
