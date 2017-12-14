#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <types.h>
#include "fs/fs.h"
#include "kernel/mm.h"
#include "kernel/mmu.h"
#include "lib/list.h"

#define PROCESS_STATE_SLEEP 0
#define PROCESS_STATE_ACTIVE 1

#define MAX_THREAD 5

#define PROCESS_READY 0
#define PROCESS_RUNING 1
#define PROCESS_FINISHED 2

typedef struct process_s process_t;

typedef struct {
    vfs_node_t **entries;
    size_t length;
    size_t capacity;
    size_t refs;
} fd_set_t;

typedef struct process_s {
    int ref;
    uint32_t id;
    uint32_t uid;
    uint32_t gid;
    uint32_t parent_id;

    int priority;
    int state;
    int interrupt;

    int flags;
    char name[32];
    int status;

    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t kstack;
    uint32_t ustack;
    size_t size;

    uint32_t img_entry;
    uint32_t img_size;

    fd_set_t *fds;

    uint32_t brk;

    vm_area_t *mmap;

    process_t *next;
    page_dir_t *pd;
    registers_t *syscall_regs;
    // thread threads[MAX_THREAD];
} process_t;

process_t *process_create(process_t *parent);

extern volatile process_t *_curr_process;

void process_init();

void context_switch();

int sys_fork();

void move_stack(uint32_t new_stack_start, uint32_t size);

void switch_to_user_mode(uint32_t location, uint32_t ustack);

int sys_exec(char *path, int argc, char **argv);

int sys_getpid();

int sleep_on(list_t *queue);
#endif
