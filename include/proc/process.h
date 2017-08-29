#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <types.h>
#include "mm/mmu.h"

#define KSTACK_SIZE 0x2000
#define USTACK_SIZE 0x10000

#define USTACK_BOTTOM 0x30000000

#define PROCESS_STATE_SLEEP 0
#define PROCESS_STATE_ACTIVE 1

#define MAX_THREAD 5

typedef struct process_s process_t;

typedef struct process_s {
    int ref;
    uint32_t id;
    uint32_t uid;
    uint32_t gid;

    int priority;
    int state;

    int flags;
    char name[32];
    int status;

    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t kstack;
    uint32_t ustack;
    size_t size;

    process_t *next;
    page_dir_t *pd;
    // thread threads[MAX_THREAD];
} process_t;

process_t *process_create(process_t *parent);

extern volatile process_t *_curr_process;

void process_init();

void context_switch();

int fork();

void move_stack(uint32_t new_stack_start, uint32_t size);

int getpid();

void switch_to_user_mode();

int say();

#endif
