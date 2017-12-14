#include "fs/fs.h"
#include "kernel.h"

vfs_node_t *create_pipe(size_t size){
    vfs_node_t *node = kmalloc(sizeof(vfs_node_t));
    memset(node,0,sizeof(vfs_node_t));
    
    node->device = 0;
    node->name[0] = '\0';
    sprintf(node->name,"[pipe]");
    node->uid = 0;
    node->gid = 0;
    node->mask = 0666;
    node->flags = VFS_PIPE;
    node->read = NULL;

    return node;
}
