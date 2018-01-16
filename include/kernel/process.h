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
#define PROCESS_SLEEP 3

#define CP _current_process

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

    uint32_t end_tick;
    uint32_t end_subtick;

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

typedef void (*tasklet_t)(void *, void *);

process_t *process_create(process_t *parent);

void process_init();
int process_spawn_tasklet(tasklet_t tasklet, char *name, void *argp);
void process_exit(int ret);
void process_wakeup_sleepers(uint32_t ticks, uint32_t subticks);

void context_switch(int reschedule);

void move_stack(uint32_t new_stack_start, uint32_t size);

void switch_to_user_mode(uint32_t location, int argc, char **argv,
                         uint32_t ustack);

int sys_fork();
int sys_exec(char *path, int argc, char **argv);
int sys_getpid();

int sleep_on(list_t *queue);

extern void return_to_userspace(void);
extern void enter_userspace(uint32_t location, uint32_t ustack);

extern process_t *_current_process;
#endif
