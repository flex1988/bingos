#ifndef __DIRENT_H__
#define __DIRENT_H__

#include <types.h>

struct __dirstream {
    int fd;
    int current_entry;
};

typedef struct __dirstream DIR;

typedef struct dirent {
    uint32_t d_ino;
    char d_name[256];
} dirent_t;

DIR* opendir(const char* name);
int closedir(DIR* dir);
dirent_t* readdir(DIR* dir);
int mkdir(const char *path, int mode);

#endif
