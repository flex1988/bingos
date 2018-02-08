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

#define EXT2_DIRECT_BLOCKS 12

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

vfs_node_t *ext2_finddir(vfs_node_t *node, char *name);

static uint32_t ext2_root(ext2_fs_t *this, ext2_inode_t *inode, vfs_node_t *fnode) {
    if (!fnode)
        return 0;

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
    if ((inode->i_mode & EXT2_S_IFREG) == EXT2_S_IFREG) {
        return 0;
    }

    if ((inode->i_mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) {
        ;
    } else {
        printk("Ext2 root node is not a directory");
        return 0;
    }

    if ((inode->i_mode & EXT2_S_IFBLK) == EXT2_S_IFBLK) {
        fnode->flags |= VFS_BLOCKDEVICE;
    }

    if ((inode->i_mode & EXT2_S_IFCHR) == EXT2_S_IFCHR) {
        fnode->flags |= VFS_CHARDEVICE;
    }

    if ((inode->i_mode & EXT2_S_IFIFO) == EXT2_S_IFIFO) {
        fnode->flags |= VFS_PIPE;
    }

    if ((inode->i_mode & EXT2_S_IFLNK) == EXT2_S_IFLNK) {
        fnode->flags |= VFS_SYMLINK;
    }

    fnode->atime = inode->i_atime;
    fnode->mtime = inode->i_mtime;
    fnode->ctime = inode->i_ctime;

    fnode->flags |= VFS_DIRECTORY;
    fnode->read = NULL;
    fnode->write = NULL;
    fnode->finddir = ext2_finddir;

    return 1;
}

static int ext2_read_block(ext2_fs_t *this, uint32_t block_no, uint8_t *buf) {
    if (!block_no) {
        return E_BADBLOCK;
    }

    vfs_read(this->block_device, block_no * this->block_size, this->block_size, (uint8_t *)buf);

    return E_SUCCESS;
}

static uint32_t ext2_create_node(ext2_fs_t *this, ext2_inode_t *inode, ext2_dir_entry_t *dentry, vfs_node_t *fnode) {
    if (!fnode) {
        return 0;
    }

    fnode->device = (void *)this;
    fnode->inode = dentry->inode;
    memcpy(&fnode->name, &dentry->name, dentry->name_len);
    fnode->name[dentry->name_len] = '\0';

    fnode->uid = inode->i_uid;
    fnode->gid = inode->i_gid;
    fnode->length = inode->i_size;
    fnode->mask = inode->i_mode & 0xfff;
    fnode->nlink = inode->i_links_count;

    return 1;
}

static uint32_t ext2_get_block_number(ext2_fs_t *this, ext2_inode_t *inode, uint32_t index) {
    uint32_t pointers = this->pointers_per_block;
    uint8_t *buf;

    // 0-11 is direct block pointer
    if (index < EXT2_DIRECT_BLOCKS) {
        return inode->i_block[index];
    } else if (index < pointers + EXT2_DIRECT_BLOCKS) {
        buf = kmalloc(this->block_size);
        ext2_read_block(this, inode->i_block[EXT2_DIRECT_BLOCKS], buf);
        uint32_t block = buf[index - EXT2_DIRECT_BLOCKS];
        kfree(buf);
        return block;
    } else if (index < pointers * pointers + EXT2_DIRECT_BLOCKS) {
        buf = kmalloc(this->block_size);
        ext2_read_block(this, inode->i_block[EXT2_DIRECT_BLOCKS + 1], buf);

        uint32_t iblock = buf[(index - (pointers + EXT2_DIRECT_BLOCKS)) / pointers];
        ext2_read_block(this, iblock, buf);

        uint32_t block = buf[index - (index - (pointers + EXT2_DIRECT_BLOCKS) / pointers) * pointers];
        kfree(buf);

        return block;
    } else if (index < pointers * pointers * pointers + EXT2_DIRECT_BLOCKS) {
        buf = kmalloc(this->block_size);
        ext2_read_block(this, inode->i_block[EXT2_DIRECT_BLOCKS + 2], buf);

        uint32_t b1 = index - EXT2_DIRECT_BLOCKS;
        uint32_t b2 = b1 - pointers;
        uint32_t b3 = b2 - pointers * pointers;
        uint32_t b4 = b3 / (pointers * pointers);
        uint32_t b5 = b3 - b4 * (pointers * pointers);

        uint32_t block4 = buf[b4];
        ext2_read_block(this, block4, buf);

        uint32_t block = buf[b5];

        kfree(buf);

        return block;
    }

    printk("Fatel error: while read inode with a too high block number");
    return 0;
}

// return [block]'th block inside in [inode] copy to [buf] and return real block number
static uint32_t ext2_inode_read_block(ext2_fs_t *this, ext2_inode_t *inode, uint32_t block, uint8_t *buf) {
    if (block > inode->i_sectors / (this->block_size / SECTORSIZE)) {
        memset(buf, 0x0, this->block_size);
        return 0;
    }

    uint32_t real = ext2_get_block_number(this, inode, block);

    ext2_read_block(this, real, buf);

    return real;
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

    memcpy(inode, (uint8_t *)((uint32_t)buf + offset_in_block * this->inode_size), this->inode_size);

    kfree(buf);
}

static ext2_inode_t *ext2_read_inode(ext2_fs_t *this, uint32_t ino) {
    ext2_inode_t *inode = kmalloc(this->inode_size);
    ext2_refresh_inode(this, inode, ino);
    return inode;
}

vfs_node_t *ext2_finddir(vfs_node_t *node, char *name) {
    ext2_fs_t *this = (ext2_fs_t *)node->device;
    ext2_inode_t *inode = ext2_read_inode(this, node->inode);

    uint8_t *block = kmalloc(this->block_size);
    uint8_t block_nr = 0;

    ext2_dir_entry_t *dir = NULL;

    ext2_inode_read_block(this, inode, block_nr, block);

    uint32_t dir_offset = 0;
    uint32_t total_offset = 0;

    while (total_offset < inode->i_size) {
        if (dir_offset >= this->block_size) {
            block_nr++;
            dir_offset -= this->block_size;
            ext2_inode_read_block(this, inode, block_nr, block);
        }

        ext2_dir_entry_t *dentry = (ext2_dir_entry_t *)((uint32_t)block + dir_offset);

        if (dentry->inode == 0 || strlen(name) != dentry->name_len) {
            dir_offset += dentry->rec_len;
            total_offset += dentry->rec_len;

            continue;
        }

        char *dname = kmalloc(dentry->name_len + 1);
        memcpy(dname, dentry->name, dentry->name_len);
        dname[dentry->name_len] = '\0';

        if (!strcmp(dname, name)) {
            kfree(dname);

            dir = kmalloc(dentry->rec_len);
            memcpy(dir, dentry, dentry->rec_len);
            break;
        }

        kfree(dname);

        dir_offset += dentry->rec_len;
        total_offset += dentry->rec_len;
    }

    kfree(inode);
    if (!dir) {
        kfree(block);
        return NULL;
    }

    vfs_node_t *target = kmalloc(sizeof(vfs_node_t));
    memset(target, 0x0, sizeof(vfs_node_t));

    inode = ext2_read_inode(this, dir->inode);

    ext2_create_node(this, inode, dir, target);

    kfree(dir);
    kfree(inode);
    kfree(block);

    return target;
}

static void ext2_print_disk_info(ext2_fs_t *this) {
    printk("-----------------------------");

    printk("total inodes count \t %d", SB->s_inodes_count);
    printk("free inodes count \t %d", SB->s_free_inodes_count);
    printk("total blocks count \t %d", SB->s_blocks_count);
    printk("free blocks count \t %d", SB->s_free_blocks_count);
    printk("reserved blocks count \t %d", SB->s_r_blocks_count);
    printk("mounteds last fsck \t %d", SB->s_mnt_count);
    printk("blocks per group \t %d", SB->s_blocks_per_group);
    printk("block number of sb \t %d", SB->s_first_data_block);
    printk("filesysten state \t %s", SB->s_state == 1 ? "clean" : "error");
    printk("Creator OS \t %d", SB->s_creator_os);

    printk("-----------------------------");
}

static vfs_node_t *ext2_do_mount(vfs_node_t *block_device, int flags) {
    ext2_fs_t *this = kmalloc(sizeof(ext2_fs_t));
    memset(this, 0x0, sizeof(ext2_fs_t));

    this->flags = flags;

    this->block_device = block_device;
    this->block_size = 1024;

    SB = kmalloc(this->block_size);

    ext2_read_block(this, 1, (uint8_t *)SB);

    if (SB->s_magic != EXT2_SUPER_MAGIC) {
        printk("Not a valid ext2 filesystem, magic does not match");
        return NULL;
    }

    this->inode_size = 128;
    this->block_size = 1024 << SB->s_log_block_size;
    this->cache_entries = 10240;

    if (this->block_size > 2048) {
        this->cache_entries /= 4;
    }

    this->pointers_per_block = this->block_size / 4;

    BGDS = SB->s_blocks_count / SB->s_blocks_per_group;

    if (SB->s_blocks_per_group * BGDS < SB->s_blocks_count) {
        BGDS += 1;
    }

    this->inodes_per_group = SB->s_inodes_count / BGDS;

    DC = NULL;
    printk("ext2 cache is disabled");

    ext2_print_disk_info(this);

    // load block group descriptors
    this->bgd_block_span = sizeof(ext2_group_desc_t) * BGDS / this->block_size + 1;
    BGD = kmalloc(this->block_size * this->bgd_block_span);

    if (this->block_size > 1024) {
        this->bgd_offset = 1;
    } else {
        this->bgd_offset = 2;
    }

    for (int i = 0; i < this->bgd_block_span; i++) {
        ext2_read_block(this, this->bgd_offset + i, (uint8_t *)((uint32_t)BGD + this->block_size * i));
    }

    // inode number start at no 1, inode of number 2 is root directory
    ext2_inode_t *root_node = ext2_read_inode(this, 2);
    RN = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    if (!ext2_root(this, root_node, RN)) {
        return NULL;
    }

    return RN;
}

static vfs_node_t *ext2_mount(char *device, char *mount_path) {
    vfs_node_t *dev = vfs_fetch_device(device);

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
