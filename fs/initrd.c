#include "fs/initrd.h"
#include "fs/fs.h"

initrd_header_t *initrd_header;
initrd_file_header_t *file_headers;
vfs_node_t *initrd_root;
vfs_node_t *root_nodes;
int nroot_nodes;

struct dirent dirent;

static uint32_t initrd_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    initrd_file_header_t header = file_headers[node->inode];
    if (offset > header.length)
        return 0;
    if (offset + size > header.length)
        size = header.length - offset;

    memcpy(buffer, (uint8_t *)(header.offset + offset), size);
    return size;
}

static dirent_t *initrd_readdir(vfs_node_t *node, uint32_t index) {
    if (index > nroot_nodes - 1)
        return 0;

    strcpy(dirent.d_name, root_nodes[index].name);
    dirent.d_name[strlen(root_nodes[index].name)] = 0;
    dirent.d_ino = root_nodes[index].inode;
    return &dirent;
}

static vfs_node_t *initrd_finddir(vfs_node_t *node, char *name) {
    for (int i = 0; i < nroot_nodes; i++) {
        if (!strcmp(name, root_nodes[i].name))
            return &root_nodes[i];
    }

    return 0;
}

vfs_node_t *initrd_init(uint32_t location) {
    initrd_header = (initrd_header_t *)location;
    file_headers = (initrd_file_header_t *)(location + sizeof(initrd_header_t));

    initrd_root = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    strcpy(initrd_root->name, "initrd");
    initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
    initrd_root->flags = VFS_DIRECTORY;
    initrd_root->read = 0;
    initrd_root->write = 0;
    initrd_root->open = 0;
    initrd_root->close = 0;
    initrd_root->readdir = &initrd_readdir;
    initrd_root->finddir = &initrd_finddir;
    initrd_root->ptr = 0;
    initrd_root->impl = 0;

    root_nodes = (vfs_node_t *)kmalloc(sizeof(vfs_node_t) * initrd_header->nfiles);
    nroot_nodes = initrd_header->nfiles;
    for (int i = 0; i < initrd_header->nfiles; i++) {
        file_headers[i].offset += location;
        strcpy(root_nodes[i].name, &file_headers[i].name);
        root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
        root_nodes[i].length = file_headers[i].length;
        root_nodes[i].inode = i;
        root_nodes[i].flags = VFS_FILE;
        root_nodes[i].read = &initrd_read;
        root_nodes[i].write = 0;
        root_nodes[i].readdir = 0;
        root_nodes[i].finddir = 0;
        root_nodes[i].open = 0;
        root_nodes[i].close = 0;
        root_nodes[i].impl = 0;
    }

    return initrd_root;
}

void ramdisk_init(multiboot_info_t *boot_info)
{
    vfs_node_t *ramdisk = initrd_init(*(uint32_t *)(boot_info->mods_addr));
    vfs_mount("/bin", ramdisk);
    printk("[Ramdisk] init ramdisk...");
}