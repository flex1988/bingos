#include "fs/fs.h"
#include <errno.h>

int namei(const char* path, vfs_node_t** res) {
    vfs_node_t *ret = NULL;
    ret = vfs_lookup(path, 1);

    if(!ret)
        return -ENOENT;
    
    *res = ret;

    return 0;
}
