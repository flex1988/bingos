#ifndef __FS_H__
#define __FS_H__

#include "types.h"

#define VFS_FILE 0x01
#define VFS_DIRECTORY 0x02
#define VFS_CHARDEVICE 0x03
#define VFS_BLOCKDEVICE 0x04
#define VFS_PIPE 0x05
#define VFS_SYMLINK 0x06
#define VFS_MOUNTPOINT 0x08

typedef struct vfs_node_s vfs_node_t;
typedef struct dirent dirent_t;

typedef vfs_node_t *(*vfs_mount_callback)(char *arg, char *mount_point);

typedef uint32_t (*read_type_t)(vfs_node_t *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*write_type_t)(vfs_node_t *, uint32_t, uint32_t, uint8_t *);
typedef void (*open_type_t)(vfs_node_t *);
typedef void (*close_type_t)(vfs_node_t *);
typedef dirent_t *(*readdir_type_t)(vfs_node_t *, uint32_t);
typedef vfs_node_t *(*finddir_type_t)(vfs_node_t *, char *name);

struct dirent {
    char name[128];
    uint32_t ino;
};

typedef struct vfs_node_s {
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

typedef struct vfs_entry_s {
    char name[64];
    vfs_node_t *file;
    char *device;
    char *type;
} vfs_entry_t;

extern vfs_node_t *vfs_root;

uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void vfs_open(vfs_node_t *node, uint8_t read, uint8_t write);
void vfs_close(vfs_node_t *node);
dirent_t *vfs_readdir(vfs_node_t *node, uint32_t index);
vfs_node_t *vfs_finddir(vfs_node_t *node, char *name);
vfs_node_t *vfs_lookup(const char *path, int type);
void vfs_init();
int vfs_register(char *name, vfs_mount_callback callback);
int vfs_mount_type(char *type, char *arg, char *mount_point);
#endif
