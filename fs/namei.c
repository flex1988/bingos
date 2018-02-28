#include <errno.h>
#include "fs/fs.h"
#include "kernel.h"

int namei(const char *path, vfs_node_t **res) {
    vfs_node_t *ret = NULL;
    ret = vfs_lookup(path, 1);

    if (!ret)
        return -ENOENT;

    *res = ret;

    return 0;
}

// 返回指定路径目录的inode，以及目录包含的name
// 比如路径/foo/bar/xxx返回bar对应的目录inode和xxx作为name
static int dir_namei(const char *pathname, int *namelen, const char **name, vfs_node_t *base, vfs_node_t **res_inode) {
    char c;
    const char *thisname;
    int len, error;
    vfs_node_t *inode;

    *res_inode = NULL;

    ASSERT(*pathname == '/');

    pathname++;
    /*base = ((vfs_entry_t *)vfs_tree->root)->file;*/

    base = vfs_get_mount_point("/", 1, NULL, NULL);
    printk("base 0x%x",base);

    while (1) {
        thisname = pathname;
        for (len = 0; (c = *(pathname++)) && (c != '/'); len++)
            ;

        if (!c)
            break;

        error = vfs_lookup_child(base, thisname, len, &inode);

        if (error) {
            return error;
        }

        base = inode;
    }

    *name = thisname;
    *namelen = len;
    *res_inode = base;

    return 0;
}

static int do_mkdir(const char *pathname, int mode) {
    const char *basename;
    int namelen, error;
    vfs_node_t *dir;

    error = dir_namei(pathname, &namelen, &basename, NULL, &dir);

    if (error)
        return error;
    if (!namelen)
        return -ENOENT;

    return vfs_mkdir(dir, basename, namelen, mode);
}

int sys_mkdir(const char *pathname, int mode) { return do_mkdir(pathname, mode); }
