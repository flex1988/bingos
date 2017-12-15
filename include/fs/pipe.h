#ifndef __PIPE_H__
#define __PIPE_H__

#include <types.h>
#include "fs/fs.h"
#include "lib/list.h"

typedef struct {
    uint8_t *buffer;
    size_t write_ptr;
    size_t read_ptr;
    size_t size;
    size_t refs;
    volatile int lock_read[2];
    volatile int lock_write[2];
    list_t *wait_queue_readers;
    list_t *wait_queue_writers;
    int dead;
    list_t *alert_waiters;
} pipe_device_t;

vfs_node_t *create_pipe(size_t size);
int pipe_size(vfs_node_t *node);
int pipe_unsize(vfs_node_t *node);

#endif
