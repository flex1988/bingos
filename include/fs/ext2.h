#ifndef __EXT2_H__
#define __EXT2_h__

#include <types.h>

typedef struct {
    uint32_t s_inodes_count;       // inodes count
    uint32_t s_blocks_count;       // blocks count
    uint32_t s_r_blocks_count;     // reserved blocks count
    uint32_t s_free_blocks_count;  // free blocks count
    uint32_t s_free_inodes_count;  // free inodes count
    uint32_t s_first_data_block;   // first data block
    uint32_t s_log_block_size;     // block size
    uint32_t s_log_frag_size;      // fragment size
    uint32_t s_blocks_per_group;   // blocks per group
    uint32_t s_frags_per_group;    // fragments per group
    uint32_t s_inodes_per_group;   // inodes per group
    uint32_t s_mtime;              // mount time
    uint32_t s_wtime;              // write time

    uint16_t s_mnt_count;  // mount count
    uint16_t s_magic;      // magic signature
    uint16_t s_state;      // file system state
    uint16_t s_errors;     // behaviour when detecting errors
    uint16_t s_pad;
    uint32_t s_lastcheck;      // time of last check
    uint32_t s_checkinterval;  // max time between checks
    uint32_t s_reserved[238];
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
