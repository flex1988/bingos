#include "fs/pipe.h"
#include "fs/fs.h"
#include "kernel.h"

static void pipe_increment_read(pipe_device_t *pipe) {
    pipe->read_ptr++;
    if (pipe->read_ptr == pipe->size)
        pipe->read_ptr = 0;
}

static size_t pipe_unread(pipe_device_t *pipe) {
    if (pipe->read_ptr == pipe->write_ptr) {
        return 0;
    }

    if (pipe->read_ptr > pipe->write_ptr) {
        return (pipe->size - pipe->read_ptr) + pipe->write_ptr;
    } else {
        return pipe->write_ptr - pipe->read_ptr;
    }
}

uint32_t read_pipe(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    pipe_device_t *pipe = (pipe_device_t *)node->device;

    if (pipe->dead) {
        return 0;
    }

    size_t collected = 0;
    while (collected == 0) {
        while (pipe_unread(pipe) > 0 && collected < size) {
            buffer[collected] = pipe->buffer[pipe->read_ptr];
            pipe_increment_read(pipe);
            collected++;
        }

        wakeup_from(pipe->wait_queue_writers);
        if (collected == 0) {
            sleep_on(pipe->wait_queue_readers);
        }
    }
}

vfs_node_t *create_pipe(size_t size) {
    vfs_node_t *node = kmalloc(sizeof(vfs_node_t));
    memset(node, 0, sizeof(vfs_node_t));

    node->device = 0;
    node->name[0] = '\0';
    sprintf(node->name, "[pipe]");
    node->uid = 0;
    node->gid = 0;
    node->mask = 0666;
    node->flags = VFS_PIPE;
    node->read = NULL;

    return node;
}
