#ifndef __EXT2_H__
#define __EXT2_h__

#include <types.h>

#define EXT2_SUPER_MAGIC 0xEF53

/* File Types */
#define EXT2_S_IFSOCK 0xC000
#define EXT2_S_IFLNK 0xA000
#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFBLK 0x6000
#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFCHR 0x2000
#define EXT2_S_IFIFO 0x1000

/* setuid, etc. */
#define EXT2_S_ISUID 0x0800
#define EXT2_S_ISGID 0x0400
#define EXT2_S_ISVTX 0x0200

/* rights */
#define EXT2_S_IRUSR 0x0100
#define EXT2_S_IWUSR 0x0080
#define EXT2_S_IXUSR 0x0040
#define EXT2_S_IRGRP 0x0020
#define EXT2_S_IWGRP 0x0010
#define EXT2_S_IXGRP 0x0008
#define EXT2_S_IROTH 0x0004
#define EXT2_S_IWOTH 0x0002
#define EXT2_S_IXOTH 0x0001

typedef struct {
    uint32_t s_inodes_count;       // total inodes count
    uint32_t s_blocks_count;       // total blocks count
    uint32_t s_r_blocks_count;     // root reserved blocks
    uint32_t s_free_blocks_count;  // free blocks count
    uint32_t s_free_inodes_count;  // free inodes count
    uint32_t s_first_data_block;   // Block number of the block containing the superblock
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;  // Last mount time
    uint32_t s_wtime;  // Last written time

    uint16_t s_mnt_count;        // Number of times the volume has been mounted since its last consistency check
    uint16_t s_max_mnt_count;    // Number of mounts allowed before a consistency check must be done
    uint16_t s_magic;            // Ext2 signature (0xef53)
    uint16_t s_state;            // File system state
    uint16_t s_errors;           // What to do when an error is detected
    uint16_t s_minor_rev_level;  // Minor portion of version

    uint32_t s_lastcheck;      // POSIX time of last consistency check
    uint32_t s_checkinterval;  // Interval (in POSIX time) between forced consistency checks
    uint32_t s_creator_os;     // Operating system ID from which the filesystem on this volume was created
    uint32_t s_rev_level;      // Major portion of version

    uint16_t s_def_resuid;  // User ID that can use reserved blocks
    uint16_t s_def_resgid;  // Group ID that can use reserved blocks

    uint32_t s_reserved[235];
} __attribute__((packed)) ext2_superblock_t;

typedef struct {
    uint32_t block_bitmap;       // Block address of block usage bitmap
    uint32_t inode_bitmap;       // Block address of inode usage bitmap
    uint32_t inode_table;        // Starting block address of inode table
    uint16_t free_blocks_count;  // Number of unallocated blocks in group
    uint16_t free_inodes_count;  // Number of unallocated inodes in group
    uint16_t used_dirs_count;    // Number of directories in group
    uint16_t pad;
    uint8_t reserved[12];
} __attribute__((packed)) ext2_group_desc_t;

typedef struct {
    uint16_t i_mode;         // file mode
    uint16_t i_uid;          // owner uid
    uint32_t i_size;         // size in bytes
    uint32_t i_atime;        // access time
    uint32_t i_ctime;        // creation time
    uint32_t i_mtime;        // modification time
    uint32_t i_dtime;        // deletion time
    uint32_t i_gid;          // group id
    uint32_t i_links_count;  // links count
    uint32_t i_sectors;       // blocks count
    uint32_t i_flags;        // file flags
    uint32_t i_reserved1;
    uint32_t i_block[15];  // pointers to blocks
    uint32_t i_version;    // file version
    uint32_t i_file_acl;   // file ACL
    uint32_t i_dir_acl;    // directory ACL
    uint32_t i_faddr;      // fragment address
    uint8_t i_frag;        // fragment number
    uint8_t i_fsize;       // fragment size
    uint16_t i_pad1;
    uint32_t i_reserved2[2];
} __attribute__((packed)) ext2_inode_t;

#define EXT2_NAME_LEN 255
typedef struct {
    uint32_t inode;
    uint16_t rec_len; //Total size of this entry (Including all subfields)
    uint16_t name_len;
    char name[EXT2_NAME_LEN];
} __attribute__((packed)) ext2_dir_entry_t;

#endif
