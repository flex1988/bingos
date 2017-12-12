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

static uint32_t ext2_root(ext2_fs_t *this, ext2_inode_t *inode, vfs_node_t *fnode) { 
    if(!fnode) return 0;

    fnode->device = (void *)this;
    fnode->inode = 2;
    fnode->name[0] = '/';
    fnode->name[1] = '\0';
    fnode->uid = inode->i_uid;
    fnode->gid = inode->i_gid;
    fnode->length = inode->i_size;
    fnode->mask = inode->i_mode & 0xfff;
    fnode->nlink = inode->i_links_count;
    
    fnode->flags = 0;
    if((inode->i_mode & EXT2_S_IFREG) == EXT2_S_IFREG) {
        return 0;
    }

    if((inode->i_mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) {
        ;      
    } else {
        printk("Ext2 root node is not a directory");
        return 0;
    }

    if((inode->i_mode & EXT2_S_IFBLK) == EXT2_S_IFBLK) {
        fnode->flags |= VFS_BLOCKDEVICE;
    }

    if((inode->i_mode & EXT2_S_IFCHR) == EXT2_S_IFCHR) {
        fnode->flags |= VFS_CHARDEVICE;
    }

    if((inode->i_mode & EXT2_S_IFIFO) == EXT2_S_IFIFO) {
        fnode->flags |= VFS_PIPE;
    }

    if((inode->i_mode & EXT2_S_IFLNK) == EXT2_S_IFLNK) {
        fnode->flags |= VFS_SYMLINK;
    }

    fnode->atime = inode->i_atime;
    fnode->mtime = inode->i_mtime;
    fnode->ctime = inode->i_ctime;
    
    fnode->flags |= VFS_DIRECTORY;
    fnode->read = NULL;
    fnode->write = NULL;

    return 1; 
}

static int ext2_read_block(ext2_fs_t *this, uint32_t block_no, uint8_t *buf) {
    if (!block_no) {
        return E_BADBLOCK;
    }

    vfs_read(this->block_device, block_no * this->block_size, this->block_size, (uint8_t *)buf);

    return E_SUCCESS;
}

static void ext2_refresh_inode(ext2_fs_t *this, ext2_inode_t *inode, uint32_t ino) {
    uint32_t group = ino / this->inodes_per_group;
    if (group > BGDS) {
        return;
    }

    uint32_t inode_table_block = BGD[group].inode_table;
    ino -= group * this->inodes_per_group;

    uint32_t block_offset = ((ino - 1) * this->inode_size) / this->block_size;
    uint32_t offset_in_block = (ino - 1) - block_offset * (this->block_size / this->inode_size);

    uint8_t *buf = kmalloc(this->block_size);
    
    ext2_read_block(this, inode_table_block + block_offset, buf);

    ext2_inode_t *inodes = (ext2_inode_t *)buf;

    memcpy(inode, (uint8_t *)((uint32_t)buf + offset_in_block * this->inode_size), this->inode_size);

    kfree(buf);
}

static ext2_inode_t *ext2_read_inode(ext2_fs_t *this, uint32_t ino) {
    ext2_inode_t *inode = kmalloc(this->inode_size);
    ext2_refresh_inode(this, inode, ino);
    return inode;
}

static vfs_node_t *ext2_do_mount(vfs_node_t *block_device, int flags) {
    ext2_fs_t *this = kmalloc(sizeof(ext2_fs_t));
    memset(this, 0x0, sizeof(ext2_fs_t));

    this->flags = flags;

    this->block_device = block_device;
    this->block_size = 1024;

    SB = kmalloc(this->block_size);

    ext2_read_block(this, 1, (uint8_t *)SB);

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
        ext2_read_block(this, this->bgd_offset + i, (uint8_t *)((uint32_t)BGD + this->block_size * i));
    }

    ext2_inode_t *root_node = ext2_read_inode(this, 2);
    RN = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    if (!ext2_root(this, root_node, RN)) {
        return NULL;
    }

    return RN;
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
