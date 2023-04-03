#include "drivers/ata.h"
#include "kernel.h"
#include "fs/devfs.h"
#include "fs/ext2.h"
#include "fs/fs.h"

static vfs_node_t  ext2_root;
static vfs_node_t* block_device;
static ext2_fs_t*  ext2fs;
static dirent_t tmpdir;
#define BOOT_SECTOR_SIZE 1024
#define BLOCK_SIZE  1024
#define EXT2_DIRECT_BLOCKS 12

/* File Types */
#define EXT2_S_IFSOCK	0xC000
#define EXT2_S_IFLNK	0xA000
#define EXT2_S_IFREG	0x8000
#define EXT2_S_IFBLK	0x6000
#define EXT2_S_IFDIR	0x4000
#define EXT2_S_IFCHR	0x2000
#define EXT2_S_IFIFO	0x1000

static int ext2_read_block(uint32_t block_no, uint8_t *buf)
{
    if (block_no == 0)
    {
        return -1;
    }
    vfs_read(block_device, BOOT_SECTOR_SIZE + (block_no - 1) * BLOCK_SIZE, BLOCK_SIZE, buf);
    return 0;
}

static ext2_inode_t* ext2_read_inode(uint32_t ino)
{
    ext2_inode_t* inode = kmalloc(sizeof(ext2_inode_t));
    ino--;
    uint32_t group = ino / ext2fs->inodes_per_group;
    if (group > ext2fs->block_group_count)
    {
        PANIC("[EXT2] invalid inode number");
    }
    uint32_t inode_table_block = ext2fs->block_groups[group].inode_table;
    uint32_t ioffset = ino % ext2fs->inodes_per_group;
    uint32_t block_index = (ioffset * ext2fs->inode_size) / ext2fs->block_size;
    uint32_t block_offset = ino % (ext2fs->block_size / ext2fs->inode_size);
    char* buf = kmalloc(ext2fs->block_size);
    ext2_read_block(inode_table_block + block_index, buf);
    memcpy(inode, buf + block_offset * ext2fs->inode_size, ext2fs->inode_size);
    kfree(buf);
    return inode;
}

static uint32_t ext2_get_block_number(ext2_inode_t* inode, uint32_t block)
{
    uint32_t p = ext2fs->pointers_per_block;
    if (block < EXT2_DIRECT_BLOCKS)
    {
        return inode->i_block[block];
    }
    else
    {
        printk("[EXT2] not implement");
        return -1;
    }
}

static int ext2_inode_read_block(ext2_inode_t* inode, uint32_t block, char* buf)
{
    uint32_t real_block = ext2_get_block_number(inode, block);
    ext2_read_block(real_block, buf);
    return real_block;
}

static ext2_dir_entry_t* ext2_direntry(ext2_inode_t* inode, uint32_t ino, uint32_t index)
{
    char* buf = kmalloc(ext2fs->block_size);
    int block_index = 0;
    ext2_inode_read_block(inode, block_index, buf);

    uint32_t dir_offset = 0;
    uint32_t total_offset = 0;
    uint32_t dir_index = 0;
    while (total_offset < inode->i_size && dir_index <= index)
    {
        ext2_dir_entry_t* entry = (ext2_dir_entry_t*)(buf + dir_offset);
        if (entry->inode != 0 && dir_index == index)
        {
            ext2_dir_entry_t* out = kmalloc(entry->rec_len);
            memcpy(out, entry, entry->rec_len);
            kfree(buf);
            return out;
        }
        dir_offset += entry->rec_len;
        total_offset += entry->rec_len;
        if (entry->inode)
        {
            dir_index++;
        }
        if (dir_offset >= ext2fs->block_size)
        {
            block_index++;
            dir_offset -= ext2fs->block_size;
            ext2_inode_read_block(inode, block_index, buf);
        }
    }
    kfree(buf);
    return NULL;
}

static dirent_t* ext2_readdir(vfs_node_t* node, uint32_t index)
{
    ext2_inode_t* inode = ext2_read_inode(node->inode);
    ext2_dir_entry_t* dir = ext2_direntry(inode, node->inode, index);
    if (!dir)
    {
        kfree(inode);
        return NULL;
    }
    memcpy(tmpdir.d_name, dir->name, dir->name_len);
    tmpdir.d_name[dir->name_len] = '\0';
    tmpdir.d_ino = dir->inode;
    kfree(dir);
    kfree(inode);
    return &tmpdir;
}

static vfs_node_t *ext2_finddir(vfs_node_t *node, char *name)
{
    ext2_inode_t* inode = ext2_read_inode(node->inode);
    char* buf = kmalloc(ext2fs->block_size);
    int block_index = 0;
    ext2_inode_read_block(inode, block_index, buf);

    uint32_t dir_offset = 0;
    uint32_t total_offset = 0;
    while (total_offset < inode->i_size)
    {
        ext2_dir_entry_t* entry = (ext2_dir_entry_t*)(buf + dir_offset);
        dir_offset += entry->rec_len;
        total_offset += entry->rec_len;
        if (entry->inode != 0 && memcmp(entry->name, name, entry->name_len) == 0)
        {
            vfs_node_t* n = kmalloc(sizeof(vfs_node_t));
            n->inode = entry->inode;
            ext2_inode_t* i = ext2_read_inode(n->inode);
            if (i->i_mode & EXT2_S_IFDIR)
            {
                n->flags |= VFS_DIRECTORY;
            }
            if (i->i_mode & EXT2_S_IFREG)
            {
                n->flags |= VFS_FILE;
            }
            n->readdir = ext2_readdir;
            n->finddir = ext2_finddir;

            memcpy(n->name, entry->name, strlen(entry->name_len + 1));
            kfree(inode);
            return n;
        }
        if (dir_offset >= ext2fs->block_size)
        {
            dir_offset -= ext2fs->block_size;
            block_index++;
            ext2_inode_read_block(inode, block_index, buf);
        }
    }
    kfree(inode);
    kfree(buf);

    return NULL;
}

static vfs_node_t* ext2_mount(vfs_node_t* root, int flags)
{
    block_device = root;
    ext2_root.inode = 2;
    ext2_root.name[0] = '/';
    ext2_root.name[1] = '\0';
    ext2_root.flags = 0;
    ext2_root.flags |= VFS_BLOCKDEVICE;
    ext2_root.flags |= VFS_CHARDEVICE;
    ext2_root.flags |= VFS_DIRECTORY;
    ext2_root.finddir = ext2_finddir;
    ext2_root.readdir = ext2_readdir;
    ext2_root.read = NULL;
    ext2_root.write = NULL;

    ext2_superblock_t* super_block = ext2fs->superblock;
    ext2_read_block(1, super_block);

    printk("Total inodes count \t %d", super_block->s_inodes_count);
    printk("Total blocks count \t %d", super_block->s_blocks_count);
    printk("Root reserved blocks count \t %d", super_block->s_r_blocks_count);
    printk("Free blocks count \t %d", super_block->s_free_blocks_count);
    printk("Free inodes count \t %d", super_block->s_free_inodes_count);
    printk("Log_block_size \t %d", super_block->s_log_block_size);
    printk("Log_frag_size \t %d", super_block->s_log_frag_size);
    printk("Blocks_per_group \t %d", super_block->s_blocks_per_group);
    printk("Frags_per_group \t %d", super_block->s_frags_per_group);
    printk("Inodes_per_group \t %d", super_block->s_inodes_per_group);
    printk("Block number of the block containing the superblock \t %d", super_block->s_first_data_block);
    printk("Last mount time \t %d", super_block->s_mtime);
    printk("Last written time \t %d", super_block->s_wtime);
    printk("Mount count since last check \t %d", super_block->s_mnt_count);
    printk("Max mount count allowed before a new check \t %d", super_block->s_max_mnt_count);
    printk("Filesystem state \t %d", super_block->s_state);
    printk("Minor portion of version \t %d", super_block->s_minor_rev_level);
    printk("POSIX time of last consistency check \t %d", super_block->s_lastcheck);
    printk("Interval (in POSIX time) between forced consistency checks \t %d", super_block->s_checkinterval);
    printk("Operating system ID \t %d", super_block->s_creator_os);
    printk("Major portion of version \t %d", super_block->s_rev_level);
    printk("First INO number \t %d", super_block->s_first_ino);
    printk("First inode size \t %d", super_block->s_inode_size);

    if (super_block->s_magic != EXT2_SUPER_MAGIC)
    {
        printk("[Ext2] mount ext2 failed not valid magic %d expected %d", super_block->s_magic, EXT2_SUPER_MAGIC);
        return NULL;
    }
    ext2fs->inode_size = super_block->s_inode_size == 0 ? 128 : super_block->s_inode_size;
    ext2fs->block_size = 1024 << super_block->s_log_block_size;
    ext2fs->cache_entries = 10240;
    ext2fs->pointers_per_block = ext2fs->block_size / 4;
    ext2fs->block_group_count = (super_block->s_blocks_count + super_block->s_blocks_per_group - 1) / super_block->s_blocks_per_group;
    ext2fs->inodes_per_group = super_block->s_inodes_per_group;
    // load block group descriptors
    ext2fs->bgd_block_span = sizeof(ext2_group_desc_t) * ext2fs->block_group_count / ext2fs->block_size + 1;
    ext2fs->block_groups = kmalloc(ext2fs->block_size * ext2fs->bgd_block_span);
    ext2fs->bgd_offset = 2;
    for (int i = 0; i < ext2fs->bgd_block_span; i++)
    {
        ext2_read_block(ext2fs->bgd_offset + i, (uint8_t*)ext2fs->block_groups + i * ext2fs->block_size);
    }

    for (int i = 0; i < ext2fs->block_group_count; i++)
    {
        printk("BlockGroupDescriptor %d %d", i, ext2fs->bgd_offset + i * super_block->s_blocks_per_group);
        printk("BlockBitmap %d", ((ext2_group_desc_t*)ext2fs->block_groups + i)->block_bitmap);
        printk("INodeBitmap %d", ((ext2_group_desc_t*)ext2fs->block_groups + i)->inode_bitmap);
        printk("InodeTable %d", ((ext2_group_desc_t*)ext2fs->block_groups + i)->inode_table);
        printk("FreeBlocks %d", ((ext2_group_desc_t*)ext2fs->block_groups + i)->free_blocks_count);
        printk("FreeINodes %d", ((ext2_group_desc_t*)ext2fs->block_groups + i)->free_inodes_count);
    }

    printk("Ext2 magic %d \t %d", EXT2_SUPER_MAGIC, super_block->s_magic);
    printk("Ext2 inodesize %d \t %d", sizeof(ext2_inode_t), ext2fs->inode_size);
    printk("Ext2 blocksize \t %d", ext2fs->block_size);
    printk("Ext2 pointers_per_block \t %d", ext2fs->pointers_per_block);
    printk("Ext2 block_group_count \t %d", ext2fs->block_group_count);
    printk("Ext2 inodes_per_block \t %d", ext2fs->block_size / sizeof(ext2_inode_t));
    printk("Ext2 inodes_per_group \t %d", ext2fs->inodes_per_group);
    printk("Ext2 bgd_block_span \t %d", ext2fs->bgd_block_span);
    printk("Ext2 bgd_offset \t %d", ext2fs->bgd_offset);

    ext2_group_desc_t* first = ext2fs->block_groups;
    char* buf = kmalloc(BLOCK_SIZE);
    ext2_read_block(first->inode_table, buf);
    ext2_inode_t* inode = buf;
    for (int i = 0; i < BLOCK_SIZE / sizeof(ext2_inode_t); i++)
    {
        printk("INode inode %d block %d", i, (inode + i)->i_block[0]);
    }

    // inode number start at no 1, inode of number 2 is root directory
    ext2_inode_t* ext2root = ext2_read_inode(ext2_root.inode);

    return &ext2_root;
}

static vfs_node_t *on_ext2_mount(char *device, char *mount_path)
{
    vfs_node_t *dev = devfs_fetch_device(device);
    if (!dev)
    {
        printk("[Module][ext2] not found device %s", device);
        return NULL;
    }

    return ext2_mount(dev, 0);
}

int ext2_init()
{
    printk("[Ext2] ext2 init...");
    ext2fs = kmalloc(sizeof(ext2_fs_t));
    ext2fs->superblock = kmalloc(BLOCK_SIZE);
    int ret = vfs_register("ext2", on_ext2_mount);
    return 0;
}