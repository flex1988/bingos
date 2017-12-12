#ifndef __EXT2_H__
#define __EXT2_h__

#include <types.h>

#define EXT2_SUPER_MAGIC 0xEF53

/* File Types */
#define EXT2_S_IFSOCK	0xC000
#define EXT2_S_IFLNK	0xA000
#define EXT2_S_IFREG	0x8000
#define EXT2_S_IFBLK	0x6000
#define EXT2_S_IFDIR	0x4000
#define EXT2_S_IFCHR	0x2000
#define EXT2_S_IFIFO	0x1000

/* setuid, etc. */
#define EXT2_S_ISUID	0x0800
#define EXT2_S_ISGID	0x0400
#define EXT2_S_ISVTX	0x0200

/* rights */
#define EXT2_S_IRUSR	0x0100
#define EXT2_S_IWUSR	0x0080
#define EXT2_S_IXUSR	0x0040
#define EXT2_S_IRGRP	0x0020
#define EXT2_S_IWGRP	0x0010
#define EXT2_S_IXGRP	0x0008
#define EXT2_S_IROTH	0x0004
#define EXT2_S_IWOTH	0x0002
#define EXT2_S_IXOTH	0x0001

 //typedef struct {
 //uint32_t s_inodes_count;       // inodes count
 //uint32_t s_blocks_count;       // blocks count
 //uint32_t s_r_blocks_count;     // reserved blocks count
 //uint32_t s_free_blocks_count;  // free blocks count
 //uint32_t s_free_inodes_count;  // free inodes count
 //uint32_t s_first_data_block;   // first data block
 //uint32_t s_log_block_size;     // block size
 //uint32_t s_log_frag_size;      // fragment size
 //uint32_t s_blocks_per_group;   // blocks per group
 //uint32_t s_frags_per_group;    // fragments per group
 //uint32_t s_inodes_per_group;   // inodes per group
 //uint32_t s_mtime;              // mount time
 //uint32_t s_wtime;              // write time

 //uint16_t s_mnt_count;  // mount count
 //uint16_t s_max_mnt_count;
 //uint16_t s_magic;      // magic signature
 //uint16_t s_state;      // file system state
 //uint16_t s_errors;     // behaviour when detecting errors
 //uint16_t s_pad;
 //uint32_t s_lastcheck;      // time of last check
 //uint32_t s_checkinterval;  // max time between checks
 //uint32_t s_reserved[238];
//} __attribute__((packed)) ext2_superblock_t;

typedef struct {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;

    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;

    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;

    uint16_t def_resuid;
    uint16_t def_resgid;

    /* EXT2_DYNAMIC_REV */
    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;

    uint8_t uuid[16];
    uint8_t volume_name[16];

    uint8_t last_mounted[64];

    uint32_t algo_bitmap;

    /* Performance Hints */
    uint8_t prealloc_blocks;
    uint8_t prealloc_dir_blocks;
    uint16_t _padding;

    /* Journaling Support */
    uint8_t journal_uuid[16];
    uint32_t journal_inum;
    uint32_t jounral_dev;
    uint32_t last_orphan;

    /* Directory Indexing Support */
    uint32_t hash_seed[4];
    uint8_t def_hash_version;
    uint16_t _padding_a;
    uint8_t _padding_b;

    /* Other Options */
    uint32_t default_mount_options;
    uint32_t first_meta_bg;
    uint8_t _unused[760];

} __attribute__((packed)) ext2_superblock_t;

typedef struct {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
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
    uint32_t i_blocks;       // blocks count
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
    uint16_t rec_len;
    uint16_t name_len;
    char name[EXT2_NAME_LEN];
} __attribute__((packed)) ext2_dir_entry_t;

#endif
