#include "fs/ext2.h"
#include "drivers/ata.h"
#include "fs/fs.h"
#include "kernel.h"
#include "module.h"

#define E_SUCCESS 0
#define E_BADBLOCK 1
#define E_NOSPACE 2
#define E_BADPARENT 3

#define BLOCKSIZE 1024
#define SECTORSIZE 512
#define DISK_PORT 0x1f0
#define BTOS(block) (block) * (BLOCKSIZE / SECTORSIZE)

typedef struct {
    ext2_superblock_t *superblock;
    ext2_group_desc_t *block_groups;
    vfs_node_t *root_node;
    vfs_node_t *block_device;

    uint32_t block_size;
    uint32_t pointers_per_block;
    uint32_t inodes_per_group;
    uint32_t block_group_count;

    void *disk_cache;
    uint32_t cache_entries;
    uint32_t cache_time;

    uint32_t lock[2];

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

static read_fs(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node)
        return -1;

    if (node->read) {
        uint32_t ret = node->read(node, offset, size, buffer);
        return ret;
    } else {
        return -1;
    }
}

static int read_block(ext2_fs_t *this, uint32_t block_no, uint8_t *buf) {
    if (!block_no) {
        return E_BADBLOCK;
    }

    vfs_read(this->block_device, block_no * BLOCKSIZE, this->block_size, (uint8_t *)buf);

    return E_SUCCESS;
}

static vfs_node_t *ext2_do_mount(vfs_node_t *block_device, int flags) {
    ext2_fs_t *this = kmalloc(sizeof(ext2_fs_t));
    memset(this, 0x0, sizeof(ext2_fs_t));

    this->flags = flags;

    this->block_device = block_device;
    this->block_size = 1024;

    SB = kmalloc(this->block_size);

    read_block(this, 1, (uint8_t *)SB);

    if (SB->magic != EXT2_SUPER_MAGIC) {
        printk("Not a valid ext2 filesystem, magic does not match");
        return NULL;
    }

    this->inode_size = SB->inode_size;
    if (SB->inode_size == 0) {
        this->inode_size = 128;
    }

    this->block_size = 1024 << SB->log_block_size;
    this->cache_entries = 10240;
    if (this->block_size > 2048) {
        this->cache_entries /= 4;
    }
    this->pointers_per_block = this->block_size / 4;
    BGDS = SB->blocks_count / SB->blocks_per_group;
    if (SB->blocks_per_group * BGDS < SB->blocks_count) {
        BGDS += 1;
    }
    this->inodes_per_group = SB->inodes_count / BGDS;

    DC = NULL;
    printk("ext2 cache is disabled");

    // load block group descriptors
    this->bgd_block_span = sizeof(ext2_group_desc_t) * BGDS / this->block_size + 1;
    BGD = kmalloc(this->block_size * this->bgd_block_span);

    this->bgd_offset = 2;

    if (this->block_size > 1024) {
        this->bgd_offset = 1;
    }

    for (int i = 0; i < this->bgd_block_span; i++) {
        read_block(this, this->bgd_offset + i, (uint8_t *)((uint32_t)BGD + this->block_size * i));
    }

    return NULL;
}

static vfs_node_t *ext2_mount(char *device, char *mount_path) {
    vfs_node_t *dev = vfs_create_device(device);

    if (!dev)
        return NULL;

    return ext2_do_mount(dev, 0);
}

int ext2_init(void) {
    printk("Loading ext2 module...");
    vfs_register("ext2", ext2_mount);
    return 0;
}

int ext2_deinit(void) { return 0; }

MODULE_DEFINE(ext2, ext2_init, ext2_deinit);
