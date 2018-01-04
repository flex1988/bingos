#ifndef __FCNTL_H__
#define __FCNTL_H__

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR

#define O_CREAT 0x0200
#define O_EXCL 0x0800
#define O_TRUNC 0x0400
#define O_NOCTTY 0x8000
#define O_ASYNC 0x0040
#define O_FSYNC 0x0080
#define O_SYNC O_FSYNC

#define O_APPEND 0x0008
#define O_NONBLOCK 0x0004

#endif
