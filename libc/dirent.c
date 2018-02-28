#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <syscall.h>
#include <types.h>

DIR *opendir(const char *name) {
    if (name[0] == '\0') {
        errno = ENOENT;
        return NULL;
    }

    int fd = open(name, O_RDONLY | O_TRUNC, 0);
    if (fd < 0)
        return NULL;

    DIR *dir = (DIR *)malloc(sizeof(DIR));
    dir->fd = fd;
    dir->current_entry = 0;

    return dir;
}

int closedir(DIR *dir) {
    if (dir && (dir->fd != -1)) {
        return close(dir->fd);
    } else {
        return -1;
    }
}

dirent_t *readdir(DIR *dir) {
    static dirent_t ent;
    memset(&ent, 0x0, sizeof(dirent_t));

    int ret = syscall_readdir(dir->fd, ++dir->current_entry, &ent);
    if (ret != 0) {
        memset(&ent, 0, sizeof(dirent_t));
        return NULL;
    }

    return &ent;
}

int mkdir(const char *path, int mode) {
    println("mkdir %s",path);
    return syscall_mkdir(path, mode);
}
