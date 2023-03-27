#include "fs/devfs.h"

static vfs_node_t* devfs_root;
static vfs_node_t** devs;
static int nr_dev_max;
static int nr_dev_count;
struct dirent dir_buf;

static dirent_t *devfs_readdir(vfs_node_t *node, uint32_t index) {
    if (index > nr_dev_count - 1)
    {
        return NULL;
    }

    strcpy(dir_buf.d_name, devs[index]->name);
    dir_buf.d_name[strlen(devs[index]->name)] = 0;
    dir_buf.d_ino = devs[index]->inode;
    return &dir_buf;
}

static vfs_node_t *devfs_finddir(vfs_node_t *node, char *name) {
    for (int i = 0; i < nr_dev_count; i++) {
        if (!strcmp(name, devs[i]->name))
            return devs[i];
    }

    return NULL;
}

int devfs_register(vfs_node_t* target) {
    if (nr_dev_count >= nr_dev_max)
    {
        return -1;
    }
    devs[nr_dev_count++] = target;
    return 0;
}

vfs_node_t* devfs_fetch_device(char* path)
{
    if (path[0] != '/' || path[1] != 'd' || path[2] != 'e' || path[3] != 'v' || path[4] != '/')
    {
        printk("[Devfs] invalid path %s", path);
        return NULL;
    }
    char* dev = path + 5;
    for (int i = 0; i < nr_dev_count; i++)
    {
        if (strcmp(devs[i]->name, dev) == 0)
        {
            return devs[i];
        }
    }
    return NULL;
}

void devfs_init()
{
    nr_dev_count = 0;
    nr_dev_max = 32;
    devfs_root = kmalloc(sizeof(vfs_node_t));
    strcpy(devfs_root->name, "devfs");
    devfs_root->mask = devfs_root->uid = devfs_root->gid = devfs_root->inode = devfs_root->length = 0;
    devfs_root->flags = VFS_DIRECTORY;
    devfs_root->read = 0;
    devfs_root->write = 0;
    devfs_root->open = 0;
    devfs_root->close = 0;
    devfs_root->readdir = &devfs_readdir;
    devfs_root->finddir = &devfs_finddir;
    devfs_root->ptr = 0;
    devfs_root->impl = 0;
    devs = (vfs_node_t**)kmalloc(nr_dev_max * sizeof(vfs_node_t));

    vfs_mount("/dev", devfs_root);
    printk("[Devfs] devfs init...");
}