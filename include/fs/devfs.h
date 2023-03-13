#ifndef __DEVFS_H__
#define __DEVFS_H__

#include "fs/fs.h"
#include "types.h"

int devfs_register(vfs_node_t* target);
void devfs_init();

#endif