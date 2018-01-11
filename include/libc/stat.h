#ifndef __STAT_H__
#define __STAT_H__

#include <types.h>

typedef struct stat_s {
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
} stat_t;

#define S_FILE 0x01
#define S_DIRECTORY 0x02
#define S_CHARDEVICE 0x03
#define S_BLOCKDEVICE 0x04
#define S_PIPE 0x05
#define S_SYMLINK 0x06
#define S_MOUNTPOINT 0x08

#endif
