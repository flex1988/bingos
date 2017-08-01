#ifndef __TASK_H__
#define __TASK_H__

#include <types.h>
#include "mm/mmu.h"

typedef struct task {
    int id;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    page_dir_t *pd;
    struct task *next;
    uint32_t kstack;
} task_t;

void tasking_init();

void task_switch();

int fork();

void move_stack(uint32_t new_stack_start, uint32_t size);

int getpid();

void switch_to_user_mode();

int say();
#endif
