#include "fs/ext2.h"
#include "fs/fs.h"
#include "kernel.h"
#include "module.h"

typedef struct {
    ext2_superblock_t *superblock;
    ext2_group_desc_t *block_groups;
    vfs_node_t *root_node;
    vfs_node_t *block_device;

    uint32_t block_size;
    uint32_t pointers_per_block;
    uint32_t inodes_per_group;
    uint32_t block_group_count;

    uint32_t cache_entries;
    uint32_t cache_time;

    uint8_t bgd_block_span;
    uint8_t bgd_offset;
    uint32_t inode_size;

    uint8_t *cache_data;

    int flags;
} ext2_fs_t;

#define BGDS (this->block_group_count)
#define SB (this->superblock)
#define BGD (this->block_groups)
#define RN (this->root_node)
#define DC (this->disk_cache)

static vfs_node_t *ext2_do_mount(vfs_node_t *block_device, int flags) {
    ext2_fs_t *this = kmalloc(sizeof(ext2_fs_t));
    memset(this, 0x0, sizeof(ext2_fs_t));

    this->flags = flags;

    this->block_device = block_device;
    this->block_size = 1024;

    SB = kmalloc(this->block_size);

    //read_block(this, 1, (uint8_t *)SB);
}

static vfs_node_t *ext2_mount(char *device, char *mount_path) {
    vfs_node_t *dev = vfs_create_device(device, 0);

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
