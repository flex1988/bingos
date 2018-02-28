#ifndef __FS_H__
#define __FS_H__

#include <types.h>
#include "net/socket.h"
#include "lib/tree.h"

#define NR_OPEN 256

#define VFS_FILE 0x01
#define VFS_DIRECTORY 0x02
#define VFS_CHARDEVICE 0x03
#define VFS_BLOCKDEVICE 0x04
#define VFS_PIPE 0x05
#define VFS_SYMLINK 0x06
#define VFS_MOUNTPOINT 0x08
#define VFS_SOCKET 0x09

#define O_TRUNC 0x0400
#define O_EXCL 0x0800

typedef struct vfs_node_s vfs_node_t;
typedef struct dirent dirent_t;

typedef vfs_node_t* (*vfs_mount_callback)(char* arg, char* mount_point);
typedef uint32_t (*read_type_t)(vfs_node_t*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(vfs_node_t*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_type_t)(vfs_node_t*);
typedef int (*close_type_t)(vfs_node_t*);
typedef dirent_t* (*readdir_type_t)(vfs_node_t*, uint32_t);
typedef vfs_node_t* (*finddir_type_t)(vfs_node_t*, char* name);
typedef int (*mkdir_type_t)(vfs_node_t*, const char*, int, int);

struct dirent {
    uint32_t d_ino;
    char d_name[256];
};

typedef struct vfs_node_s {
    uint32_t inode;
    char name[128];
    void* device;
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t length;
    uint32_t impl;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t nlink;
    uint32_t refs;
    uint32_t offset;
    socket_t* socket;
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    mkdir_type_t mkdir;
    vfs_node_t* ptr;
    void* ioctl;
} vfs_node_t;

typedef struct vfs_stat_s {
    uint16_t st_dev;
    uint16_t __pad1;
    uint32_t st_ino;
    uint16_t st_mode;
    uint16_t st_nlink;
    uint16_t st_uid;
    uint16_t st_gid;
    uint16_t st_rdev;
    uint16_t __pad2;
    uint32_t st_size;
    uint32_t st_blksize;
    uint32_t st_blocks;
    uint32_t st_atime;
    uint32_t __unused1;
    uint32_t st_mtime;
    uint32_t __unused2;
    uint32_t st_ctime;
    uint32_t __unused3;
    uint32_t __unused4;
    uint32_t __unused5;
} vfs_stat_t;

typedef struct vfs_entry_s {
    char name[64];
    vfs_node_t* file;
    char* device;
    char* type;
} vfs_entry_t;

extern vfs_node_t* vfs_root;
extern tree_t* vfs_tree;

uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void vfs_open(vfs_node_t* node, uint8_t flags);
int vfs_close(vfs_node_t* node);
dirent_t* vfs_readdir(vfs_node_t* node, uint32_t index);
vfs_node_t* vfs_finddir(vfs_node_t* node, char* name);
vfs_node_t* vfs_lookup(const char* path, int type);
int vfs_mkdir(vfs_node_t* dir, const char* name, int len, int mode);
void vfs_init();
int vfs_register(char* name, vfs_mount_callback callback);
int vfs_mount_type(char* type, char* arg, char* mount_point);
int vfs_lookup_child(vfs_node_t* parent, const char* name, int len, vfs_node_t** ret);

int namei(const char* path, vfs_node_t** res);
#endif
