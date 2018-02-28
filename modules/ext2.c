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

#define BLOCKBIT(n) (bg_buffer[((n) >> 3)] & (1 << (((n) % 8))))
#define BLOCKBYTE(n) (bg_buffer[((n) >> 3)])
#define SETBIT(n) (1 << (((n) % 8)))

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
dirent_t *ext2_readdir(vfs_node_t *node, uint32_t index);
void ext2_mkdir(vfs_node_t *parent, char *name, uint16_t permission);
void ext2_open(vfs_node_t *node, uint32_t flags);

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
    fnode->readdir = ext2_readdir;
    fnode->open = ext2_open;
    fnode->mkdir = ext2_mkdir;

    return 1;
}

static int ext2_rewrite_superblock(ext2_fs_t *this) {
    vfs_write(this->block_device, 1024, sizeof(ext2_superblock_t), (uint8_t *)SB);
    return E_SUCCESS;
}

static int ext2_write_block(ext2_fs_t *this, uint32_t block_no, uint8_t *buf) {
    if (!block_no) {
        printk("Attempted to write to block 0, maybe corrupted");
        return E_BADBLOCK;
    }

    if (!DC) {
        vfs_write(this->block_device, block_no * this->block_size, this->block_size, buf);
        return E_SUCCESS;
    }
}

static int ext2_read_block(ext2_fs_t *this, uint32_t block_no, uint8_t *buf) {
    if (!block_no) {
        return E_BADBLOCK;
    }

    vfs_read(this->block_device, block_no * this->block_size, this->block_size, (uint8_t *)buf);

    return E_SUCCESS;
}

static uint32_t ext2_allocate_inode(ext2_fs_t *this) {
    uint32_t node_no = 0;
    uint32_t node_offset = 0;
    uint32_t group = 0;
    uint8_t *bg_buffer = kmalloc(this->block_size);

    for (uint32_t i = 0; i < BGDS; i++) {
        if (BGD[i].free_inodes_count > 0) {
            printk("Group %d has %d free inodes", i, BGD[i].free_inodes_count);
            ext2_read_block(this, BGD[i].inode_bitmap, (uint8_t *)bg_buffer);

            while (BLOCKBIT(node_offset)) {
                node_offset++;
            }

            node_no = node_offset + i * this->inodes_per_group + 1;
            group = i;
            break;
        }
    }

    if (!node_no) {
        printk("Ran out of inodes");
        return 0;
    }

    BLOCKBYTE(node_offset) |= SETBIT(node_offset);
    ext2_write_block(this, BGD[group].inode_bitmap, (uint8_t *)bg_buffer);
    kfree(bg_buffer);

    BGD[group].free_inodes_count--;
    for (int i = 0; i < this->bgd_block_span; i++) {
        ext2_write_block(this, this->bgd_offset + i, (uint8_t *)((uint32_t)BGD + this->block_size * i));
    }

    SB->s_free_inodes_count--;
    ext2_rewrite_superblock(this);
    printk("s_free_inodes_count 0x%x %d",this->block_device->write,SB->s_free_inodes_count);

    return node_no;
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

static int ext2_write_inode(ext2_fs_t *this, ext2_inode_t *inode, uint32_t index) {
    uint32_t group = index / this->inodes_per_group;
    if (group > BGDS) {
        return E_BADBLOCK;
    }

    uint32_t inode_table_block = BGD[group].inode_table;
    index -= group * this->inodes_per_group;
    uint32_t block_offset = ((index - 1) * this->inode_size) / this->block_size;
    uint32_t offset_in_block = (index - 1) - block_offset * (this->block_size / this->inode_size);

    ext2_inode_t *inodet = kmalloc(this->block_size);
    ext2_read_block(this, inode_table_block + block_offset, (uint8_t *)inodet);

    memcpy((uint8_t *)((uint32_t)inodet + offset_in_block * this->inode_size), inode, this->inode_size);
    ext2_write_block(this, inode_table_block + block_offset, (uint8_t *)inodet);
    kfree(inodet);

    return E_SUCCESS;
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

static uint32_t ext2_allocate_block(ext2_fs_t *this) {
    uint32_t block_no = 0;
    uint32_t block_offset = 0;
    uint32_t group = 0;
    uint8_t *bg_buffer = kmalloc(this->block_size);

    for (uint32_t i = 0; i < BGDS; i++) {
        if (BGD[i].free_blocks_count > 0) {
            ext2_read_block(this, BGD[i].block_bitmap, (uint8_t *)bg_buffer);
            while (BLOCKBIT(block_offset)) {
                block_offset++;
            }

            block_no = block_offset + SB->s_blocks_per_group * i;
            group = i;
            break;
        }
    }

    if (!block_no) {
        kfree(bg_buffer);
        return 0;
    }

    BLOCKBYTE(block_offset) |= SETBIT(block_offset);
    ext2_write_block(this, BGD[group].block_bitmap, (uint8_t *)bg_buffer);

    BGD[group].free_blocks_count--;
    for (int i = 0; i < this->bgd_block_span; i++) {
        ext2_write_block(this, this->bgd_offset * i, (uint8_t *)((uint32_t)BGD + this->block_size * i));
    }

    SB->s_free_blocks_count--;
    ext2_rewrite_superblock(this);

    memset(bg_buffer, 0x0, this->block_size);
    ext2_write_block(this, block_no, bg_buffer);

    kfree(bg_buffer);

    return block_no;
}

static int ext2_allocate_inode_block(ext2_fs_t *this, ext2_inode_t *inode, uint32_t inode_no, uint32_t block) {
    uint32_t block_no = ext2_allocate_block(this);

    if (!block_no)
        return E_NOSPACE;

    return E_SUCCESS;
}

static uint32_t ext2_inode_write_block(ext2_fs_t *this, ext2_inode_t *inode, uint32_t inode_no, uint32_t block,
                                       uint8_t *buf) {
    if (block >= inode->i_sectors / (this->block_size / 512)) {
        ;
    }

    char *empty = NULL;
    while (block >= inode - inode->i_sectors / (this->block_size / 512)) {
        ext2_allocate_inode_block(this, inode, inode_no, inode->i_sectors / (this->block_size / 512));
        ext2_refresh_inode(this, inode, inode_no);
    }

    if (empty)
        kfree(empty);

    uint32_t real_block = ext2_get_block_number(this, inode, block);
    ext2_write_block(this, real_block, buf);

    return real_block;
}

static int ext2_create_entry(vfs_node_t *parent, char *name, uint32_t inode) {
    ext2_fs_t *this = (ext2_fs_t *)parent->device;
    ext2_inode_t *pinode = ext2_read_inode(this, parent->inode);
    if (((pinode->i_mode & EXT2_S_IFDIR) == 0) || (name == NULL)) {
        return E_BADPARENT;
    }

    uint32_t rec_len = sizeof(ext2_dir_entry_t) + strlen(name);
    rec_len += (rec_len % 4) ? (4 - (rec_len % 4)) : 0;

    uint8_t *block = kmalloc(this->block_size);
    uint8_t block_nr = 0;
    uint32_t dir_offset = 0;
    uint32_t total_offset = 0;
    int modify_or_replace = 0;
    ext2_dir_entry_t *previous;

    ext2_inode_read_block(this, pinode, block_nr, block);
    while (total_offset <= pinode->i_size) {
        if (dir_offset >= this->block_size) {
            block_nr++;
            dir_offset -= this->block_size;
            ext2_inode_read_block(this, pinode, block_nr, block);
        }
        ext2_dir_entry_t *d_ent = (ext2_dir_entry_t *)((uint32_t)block + dir_offset);

        uint32_t sreclen = d_ent->name_len + sizeof(ext2_dir_entry_t);
        sreclen += (sreclen % 4) ? (4 - (sreclen % 4)) : 0;

        {
            char f[d_ent->name_len + 1];
            memcpy(f, d_ent->name, d_ent->name_len);
            f[d_ent->name_len] = 0;
        }

        if (d_ent->rec_len != sreclen && total_offset + d_ent->rec_len == pinode->i_size) {
            dir_offset += sreclen;
            total_offset += sreclen;

            modify_or_replace = 1;
            previous = d_ent;

            break;
        }

        if (d_ent->inode == 0) {
            modify_or_replace = 2;
        }

        if (d_ent->rec_len == 0)
            break;

        dir_offset += d_ent->rec_len;
        total_offset += d_ent->rec_len;
    }

    if (!modify_or_replace) {
        ;
    }

    if (modify_or_replace == 1) {
        ;
    }

    ext2_dir_entry_t *d_ent = (ext2_dir_entry_t *)((uint32_t)block + dir_offset);

    d_ent->inode = inode;
    d_ent->rec_len = this->block_size - dir_offset;
    d_ent->name_len = strlen(name);
    memcpy(d_ent->name, name, strlen(name));

    ext2_inode_write_block(this, pinode, parent->inode, block_nr, block);

    kfree(block);
    kfree(pinode);

    return E_NOSPACE;
}

void ext2_mkdir(vfs_node_t *parent, char *name, uint16_t permission) {
    printk("ext2_mkdir");
    if (!name)
        return;

    ext2_fs_t *this = parent->device;

    vfs_node_t *check = ext2_finddir(parent, name);

    if (check) {
        printk("A file already exists");
        kfree(check);
        return;
    }

    // allocate an inode
    uint32_t inode_no = ext2_allocate_inode(this);
    printk("allocate inode 0x%x",inode_no);
    ext2_inode_t *inode = ext2_read_inode(this, inode_no);
    inode->i_atime = now();
    inode->i_ctime = inode->i_atime;
    inode->i_mtime = inode->i_atime;
    inode->i_dtime = 0;

    memset(inode->i_block, 0x0, sizeof(inode->i_block));
    inode->i_sectors = 0;
    inode->i_size = 0;
    inode->i_uid = 0;
    inode->i_gid = 0;

    inode->i_links_count = 2;
    inode->i_flags = 0;
    inode->i_mode = EXT2_S_IFDIR;

    inode->i_size = this->block_size;
    ext2_write_inode(this, inode, inode_no);

    ext2_create_entry(parent, name, inode_no);

    printk("i_atime 0x%x", inode->i_atime);
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

static ext2_dir_entry_t *ext2_direntry(ext2_fs_t *this, ext2_inode_t *inode, uint32_t no, uint32_t index) {
    uint8_t *block = kmalloc(this->block_size);
    uint8_t block_nr = 0;

    ext2_inode_read_block(this, inode, block_nr, block);

    uint32_t dir_offset = 0;
    uint32_t total_offset = 0;
    uint32_t dir_index = 0;

    while (total_offset < inode->i_size && dir_index <= index) {
        ext2_dir_entry_t *d_ent = (ext2_dir_entry_t *)((uint32_t)block + dir_offset);

        if (d_ent->inode != 0 && dir_index == index) {
            ext2_dir_entry_t *out = kmalloc(d_ent->rec_len);
            memcpy(out, d_ent, d_ent->rec_len);
            kfree(block);
            return out;
        }

        dir_offset += d_ent->rec_len;
        total_offset += d_ent->rec_len;

        if (d_ent->inode) {
            dir_index++;
        }

        if (dir_offset >= this->block_size) {
            block_nr++;
            dir_offset -= this->block_size;
            ext2_inode_read_block(this, inode, block_nr, block);
        }
    }

    kfree(block);
    return NULL;
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
            if (dentry->rec_len == 0)
                break;

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

dirent_t *ext2_readdir(vfs_node_t *node, uint32_t index) {
    ext2_fs_t *this = (ext2_fs_t *)node->device;
    ext2_inode_t *inode = ext2_read_inode(this, node->inode);
    ext2_dir_entry_t *direntry = ext2_direntry(this, inode, node->inode, index);
    if (!direntry) {
        kfree(inode);
        return NULL;
    }

    dirent_t *dirent = kmalloc(sizeof(struct dirent));
    memcpy(&dirent->d_name, &direntry->name, direntry->name_len);
    dirent->d_name[direntry->name_len] = '\0';
    dirent->d_ino = direntry->inode;
    kfree(direntry);
    kfree(inode);

    return dirent;
}

void ext2_open(vfs_node_t *node, uint32_t flags) {
    ext2_fs_t *this = node->device;

    if (flags & O_TRUNC) {
        ext2_inode_t *inode = ext2_read_inode(this, node->inode);
        inode->i_size = 0;
        ext2_write_inode(this, inode, node->inode);
    }
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
    printk("Load ext2fs");

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
