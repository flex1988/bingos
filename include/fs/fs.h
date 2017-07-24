#ifndef __FS_H__
#define __FS_H__

typedef struct {
    char name[128];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint32_t impl;
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    vfs_node_t *ptr;
} vfs_node_t;

typedef uint32_t (*read_type_t)(vfs_node_t *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*write_type_t)(struct vfs_node_t *, uint32_t, uint32_t, uint8_t *);

struct dirent {
    char name[128];
    uint32_t ino;
};
#endif
