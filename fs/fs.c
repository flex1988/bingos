#include "fs/fs.h"
#include "kernel.h"

vfs_node_t* vfs_root = 0;

static vfs_node_t* vfs_lookup_internal(vfs_node_t* n, char* path) {
    char* dir = NULL;
    int ret;

    while (path[0] == '/') path++;

    if (!path[0])
        return n;

    while (1) {
        dir = path;

        if (path) {
            path = strchr(path, '/');
            if (path) {
                path[0] = '\0';
                path++;
            }
        }

        if (!dir) {
            return n;
        }

        n = vfs_finddir(n, dir);
    }
}

uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->read)
        return node->read(node, offset, size, buffer);
    else
        return 0;
}

uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->write)
        return node->write(node, offset, size, buffer);
    return 0;
}

void vfs_open(vfs_node_t* node, uint8_t read, uint8_t write) {
    if (node->open)
        return node->open(node);
}

void vfs_close(vfs_node_t* node) {
    if (node->close)
        return node->close(node);
}

dirent_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if ((node->flags & 0x7) == VFS_DIRECTORY && node->readdir)
        return node->readdir(node, index);
    else
        return 0;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, char* name) {
    if ((node->flags & 0x7) == VFS_DIRECTORY && node->finddir)
        return node->finddir(node, name);
    else
        return 0;
}

vfs_node_t* vfs_lookup(const char* path, int type) {
    char* dup = NULL;
    vfs_node_t* n = NULL;
    vfs_node_t* c = NULL;

    // support absolute path only
    ASSERT(path[0] == '/');

    c = vfs_root;
    // ref

    dup = kmalloc(strlen(path) + 1);

    strcpy(dup, path);

    n = vfs_lookup_internal(c, dup);

    kfree(dup);

    return n;
}
