#include "fs/fs.h"

static copy_stat(vfs_node_t *node, vfs_stat_t *statbuf) {
    // statbuf->st_dev = ;
    statbuf->st_ino = 1;
    statbuf->st_uid = node->uid;
    statbuf->st_gid = node->gid;
    statbuf->st_atime = node->atime;
    statbuf->st_mtime = node->mtime;
    statbuf->st_ctime = node->ctime;
}

int sys_stat(char *filename, vfs_stat_t *statbuf) {
    vfs_node_t *node;
    int error;

    error = namei(filename, &node);
    if (error)
        return error;

    copy_stat(node, statbuf);

    // iput(node);

    return 0;
}
