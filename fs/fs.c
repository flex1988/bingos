#include "fs/fs.h"

vfs_node_t *vfs_root = 0;

uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (node->read)
        return node->read(node, offset, size, buffer);
    else
        return 0;
}

uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (node->write)
        return node->write(node, offset, size, buffer);
    return 0;
}

void vfs_open(vfs_node_t *node, uint8_t read, uint8_t write) {
    if (node->open)
        return node->open(node);
}

void vfs_close(vfs_node_t *node) {
    if (node->close)
        return node->close(node);
}

dirent_t *vfs_readdir(vfs_node_t *node, uint32_t index) {
    if ((node->flags & 0x7) == VFS_DIRECTORY && node->readdir)
        return node->readdir(node, index);
    else
        return 0;
}

vfs_node_t *vfs_finddir(vfs_node_t *node, char *name) {
    if ((node->flags & 0x7) == VFS_DIRECTORY && node->finddir)
        return node->finddir(node, name);
    else
        return 0;
}
