#ifndef __DEVFS_H__
#define __DEVFS_H__

#include "fs/fs.h"
#include "types.h"

int devfs_register(vfs_node_t* target);
vfs_node_t* devfs_fetch_device(char* path);
void devfs_init();

#endif