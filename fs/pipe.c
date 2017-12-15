#include "fs/pipe.h"
#include "fs/fs.h"
#include "kernel.h"

static void pipe_increment_read(pipe_device_t *pipe) {
    pipe->read_ptr++;
    if (pipe->read_ptr == pipe->size)
        pipe->read_ptr = 0;
}

static void pipe_increment_write(pipe_device_t *pipe) {
    pipe->write_ptr++;
    if (pipe->write_ptr == pipe->size) {
        pipe->write_ptr = 0;
    }
}

static size_t pipe_available(pipe_device_t *pipe) {
    if (pipe->read_ptr == pipe->write_ptr) {
        return pipe->size - 1;
    }

    if (pipe->read_ptr > pipe->write_ptr) {
        return pipe->read_ptr - pipe->write_ptr - 1;
    } else {
        return (pipe->size - pipe->write_ptr) + pipe->read_ptr - 1;
    }
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

    return collected;
}

uint32_t write_pipe(vfs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    pipe_device_t *pipe = (pipe_device_t *)node->device;

    if (pipe->dead) {
        return 0;
    }

    size_t written = 0;
    while (written < size) {
        while (pipe_available(pipe) > 0 && written < size) {
            pipe->buffer[pipe->write_ptr] = buffer[written];
            pipe_increment_write(pipe);
            written++;
        }

        wakeup_from(pipe->wait_queue_readers);
        if (written < size) {
            sleep_on(pipe->wait_queue_writers);
        }
    }

    return written;
}

void open_pipe(vfs_node_t *node, uint32_t flags) {
    pipe_device_t *pipe = (pipe_device_t *)node->device;
    pipe->refs++;

    return;
}

void close_pipe(vfs_node_t *node) {
    pipe_device_t *pipe = (pipe_device_t *)node->device;

    pipe->refs--;

    return;
}

vfs_node_t *create_pipe(size_t size) {
    vfs_node_t *node = kmalloc(sizeof(vfs_node_t));
    memset(node, 0, sizeof(vfs_node_t));

    pipe_device_t *pipe = kmalloc(sizeof(pipe_device_t));
    memset(pipe, 0, sizeof(pipe_device_t));

    pipe->buffer = kmalloc(size);
    pipe->write_ptr = 0;
    pipe->read_ptr = 0;
    pipe->size = size;
    pipe->refs = 0;
    pipe->dead = 0;
    pipe->wait_queue_writers = list_create();
    pipe->wait_queue_readers = list_create();

    node->device = 0;
    node->name[0] = '\0';
    sprintf(node->name, "[pipe]");
    node->uid = 0;
    node->gid = 0;
    node->mask = 0666;
    node->flags = VFS_PIPE;
    node->read = read_pipe;
    node->write = write_pipe;
    node->open = open_pipe;
    node->close = close_pipe;

    node->device = pipe;

    return node;
}
