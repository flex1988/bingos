#include "kernel/process.h"
#include "fs/fs.h"
#include "kernel.h"
#include "kernel/elf.h"
#include "kernel/kheap.h"
#include "kernel/memlayout.h"
#include "kernel/mmu.h"
#include "kernel/sched.h"
#include "lib/list.h"

#include <errno.h>

extern page_dir_t *_kernel_pd;
extern page_dir_t *_current_pd;
extern void page_map(page_t *, int, int);
extern uint32_t _initial_esp;
extern uint32_t read_eip();
extern volatile list_t *_sleep_queue;

process_t *_current_process;

process_t *_init_process = NULL;

uint32_t _next_pid = 1;

#define PUSH(stack, type, item) \
    stack -= sizeof(type);      \
    *((type *)stack) = item

void cmp_page_dir(page_dir_t *a, page_dir_t *b) {
    int i;
    for (i = 0; i < 1024; i++) {
        if (b->tabls[i])
            printk("%d", i);
        if (a->tabls[i] != b->tabls[i])
            printk("tabls %d 0x%x 0x%x", i, a->tabls[i], b->tabls[i]);
    }
}

void travse_dir(page_dir_t *dir) {
    int i;
    for (i = 0; i < 1024; i++) {
        if (dir->tabls[i])
            printk("tabls %d 0x%x", i, dir->tabls[i]);
    }

    printk("");
}

int dup_mmap(process_t *p, process_t *parent) {
    vm_area_t *i, **ptr, *tmp;

    ptr = &p->mmap;

    for (i = parent->mmap; i != NULL; i = i->next) {
        tmp = (vm_area_t *)kmalloc(sizeof(vm_area_t));
        if (!tmp)
            return -ENOMEM;

        *tmp = *i;
        tmp->next = NULL;

        *ptr = tmp;
        ptr = &tmp->next;
    }

    return 0;
}

// !!! p->kstack point to stack bottom (really important)
process_t *process_create(process_t *parent) {
    process_t *p = (process_t *)kmalloc(sizeof(process_t));
    p->id = _next_pid++;
    p->uid = 500;
    p->gid = 500;
    p->next = 0;
    memcpy(p->name, "[default]", 10);

    p->esp = 0;
    p->ebp = 0;
    p->eip = 0;

    p->state = PROCESS_READY;

    p->end_tick = 0;
    p->end_subtick = 0;

    if (parent) {
        p->kstack = kmalloc(KSTACK_SIZE) + KSTACK_SIZE;
        memset((void *)(p->kstack - KSTACK_SIZE), 0, KSTACK_SIZE);

        p->ustack = parent->ustack;

        p->brk = parent->brk;
        p->img_entry = parent->img_entry;
        p->img_size = parent->img_size;

        p->mmap = NULL;

        /*printk("parent id %d", parent->id);*/
        /*dump_vm_area(parent->mmap);*/

        dup_mmap(p, parent);

        /*printk("child id %d", p->id);*/
        /*dump_vm_area(p->mmap);*/

        p->fds = kmalloc(sizeof(fd_set_t));
        p->fds->refs = 1;
        p->fds->length = parent->fds->length;
        p->fds->capacity = parent->fds->capacity;
        p->fds->entries = kmalloc(sizeof(vfs_node_t *) * p->fds->capacity);

        for (uint32_t i = 0; i < parent->fds->length; i++) {
            p->fds->entries[i] = vfs_clone(parent->fds->entries[i]);
        }
    } else {
        p->brk = 0;
        p->ustack = 0;
        p->mmap = 0;

        p->fds = kmalloc(sizeof(fd_set_t));
        p->fds->refs = 1;
        p->fds->length = 0;
        p->fds->capacity = NR_OPEN;
        p->fds->entries = kmalloc(sizeof(vfs_node_t *) * p->fds->capacity);
    }

    p->status = 0;

    return p;
}

void process_spawn_tasklet(tasklet_t tasklet, char *name, void *argp) {
    IRQ_OFF;

    uint32_t esp, ebp;

    page_dir_t *dir = _kernel_pd;
    process_t *new = process_create(CP);
    new->pd = dir;

    registers_t regs;
    memcpy(&regs, CP->syscall_regs, sizeof(registers_t));
    new->syscall_regs = &regs;

    esp = new->kstack;
    ebp = esp;

    PUSH(esp, uint32_t, (uint32_t) "xxxx");
    PUSH(esp, uint32_t, (uint32_t)argp);

    new->esp = esp;
    new->ebp = ebp;
    new->eip = (uint32_t)tasklet;

    sched_enqueue(new);

    IRQ_OFF;

    return new->id;
}

void process_sleep_until(process_t *process, uint32_t seconds,
                         uint32_t subseconds) {
    uint32_t s, ss;

    relative_time(seconds, subseconds, &s, &ss);
    sleep_enqueue(process, s, ss);

    context_switch(CP == process ? 0 : 1);
}

void process_exit(int ret) {
    IRQ_OFF;

    ASSERT(_current_process);

    // init process should not exit
    ASSERT(_current_process->id != 1);

    _current_process->status = ret;
    _current_process->state = PROCESS_FINISHED;

    sched_enqueue_finished(_current_process);

    switch_to_next();
}

void switch_to_user_mode(uint32_t location, int argc, char **argv,
                         uint32_t ustack) {
    IRQ_OFF;
    set_kernel_stack(_current_process->kstack);

    PUSH(ustack, uint32_t, argv);
    PUSH(ustack, int, argc);

    enter_userspace(location, ustack);
}

void relocate_stack(uint32_t nstack, uint32_t ostack, uint32_t size) {
    uint32_t i, tmp, off;

    if (nstack > ostack) {
        off = nstack - ostack;
    } else {
        off = ostack - nstack;
    }

    for (i = nstack; i > nstack - size; i -= 4) {
        tmp = *(uint32_t *)i;
        if (tmp < ostack && tmp > ostack - size) {
            uint32_t *tmp2;

            if (nstack > ostack) {
                tmp += off;
            } else {
                tmp -= off;
            }

            tmp2 = (uint32_t *)i;
            *tmp2 = tmp;
        }
    }
}

void move_stack(uint32_t new_stack_start, uint32_t size) {
    uint32_t i;

    for (i = new_stack_start - size; i < new_stack_start + 0x1000;
         i += 0x1000) {
        page_t *p = get_page(i, 1, _current_pd);
        page_map(p, 0, 1);
    }

    // flush the TLB by reading and writing the page directory address again
    uint32_t pd_addr;
    __asm__ __volatile__("mov %%cr3, %0" : "=r"(pd_addr));
    __asm__ __volatile__("mov %0, %%cr3" ::"r"(pd_addr));

    uint32_t old_stack_pointer;
    __asm__ __volatile__("mov %%esp, %0" : "=r"(old_stack_pointer));
    uint32_t old_base_pointer;
    __asm__ __volatile__("mov %%ebp, %0" : "=r"(old_base_pointer));

    uint32_t offset = (uint32_t)new_stack_start - _initial_esp;

    uint32_t new_stack_pointer = old_stack_pointer + offset;
    uint32_t new_base_pointer = old_base_pointer + offset;

    memcpy((void *)new_stack_pointer, (void *)old_stack_pointer,
           _initial_esp - old_stack_pointer);

    for (i = (uint32_t)new_stack_start; i > (uint32_t)new_stack_start - size;
         i -= 4) {
        uint32_t tmp = *(uint32_t *)i;
        if ((old_stack_pointer < tmp) && (tmp < _initial_esp)) {
            tmp = tmp + offset;
            uint32_t *tmp2 = (uint32_t *)i;
            *tmp2 = tmp;
        }
    }

    __asm__ __volatile__("mov %0, %%esp" ::"r"(new_stack_pointer));
    __asm__ __volatile__("mov %0, %%ebp" ::"r"(new_base_pointer));
    printk("relocate esp 0x%x, ebp 0x%x", new_stack_pointer, new_base_pointer);
}

void process_init() {
    IRQ_OFF;

    // relocate the stack to 0xe0000000
    move_stack(KINIT_STACK_BOTTOM, KINIT_STACK_SIZE);

    sched_init();

    process_t *init = process_create(0);

    init->pd = _current_pd;
    init->kstack = KINIT_STACK_BOTTOM;
    init->state = PROCESS_RUNING;

    _current_process = _init_process = init;

    IRQ_ON;
}

int sys_fork() {
    IRQ_OFF;

    _current_process->syscall_regs->eax = 0;

    uint32_t magic = 0xEEEEEEEE;
    uint32_t esp, ebp, eip;
    process_t *parent = (process_t *)_current_process;
    page_dir_t *ppd = page_dir_clone(parent->pd);

    process_t *new = process_create(parent);

    new->pd = ppd;

    registers_t regs;
    memcpy(&regs, _current_process->syscall_regs, sizeof(registers_t));
    new->syscall_regs = &regs;

    esp = new->kstack;
    ebp = esp;
    new->syscall_regs->eax = 0;
    PUSH(esp, registers_t, regs);

    new->esp = esp;
    new->ebp = ebp;
    new->eip = (uint32_t)&return_to_userspace;

    sched_enqueue(new);

    IRQ_ON;
    return new->id;
}

void context_switch(int reschedule) {
    if (!_current_process) {
        return;
    }

    if (!sched_available()) {
        return;
    }

    uint32_t esp, ebp, eip;
    __asm__ __volatile__("mov %%esp, %0" : "=r"(esp));
    __asm__ __volatile__("mov %%ebp, %0" : "=r"(ebp));

    eip = read_eip();
    // hack magic
    if (eip == 0x12345678) {
        IRQ_ON;
        return;
    }

    IRQ_OFF;

    ASSERT(_current_process);

    _current_process->eip = eip;
    _current_process->esp = esp;
    _current_process->ebp = ebp;

    if (reschedule) {
        sched_enqueue(_current_process);
    }

    switch_to_next();
}

void switch_to_next() {
    uint32_t esp, ebp, eip;

    _current_process = sched_dequeue();

    ASSERT(_current_process);

    eip = _current_process->eip;
    esp = _current_process->esp;
    ebp = _current_process->ebp;

    ASSERT(eip != 0x12345678);

    set_kernel_stack(_current_process->kstack);

    _current_pd = _current_process->pd;

    __asm__ __volatile__(
        "mov %0, %%ecx;       \
         mov %1, %%esp;       \
         mov %2, %%ebp;       \
         mov %3, %%cr3;       \
         mov $0x12345678, %%eax; \
         sti;                   \
         jmp *%%ecx" ::"r"(eip),
        "r"(esp), "r"(ebp), "r"(_current_pd->physical)
        : "%ecx", "%esp", "%eax");
}

static void release_directory(page_dir_t *dir) {
    uint32_t i;
    for (i = 0; i < 1024; i++) {
        if (!dir->tabls[i]) {
            continue;
        }
        if (_kernel_pd->tabls[i] != dir->tabls[i]) {
            for (uint32_t j = 0; j < 1024; j++) {
                if (dir->tabls[i]->pages[j].addr) {
                    free_frame(dir->tabls[i]->pages[j].addr);
                }
            }
            kfree(dir->tabls[i]);
        }
    }
    kfree(dir);
}

void release_process(process_t *p) {
    kfree(p->fds->entries);
    kfree(p->fds);
    kfree(p->kstack - KSTACK_SIZE);
    release_directory(p->pd);
    kfree(p);
}

int sys_getpid() {
    if (!_current_process)
        return 0;
    return _current_process->id;
}

int sys_waitpid(int pid) {
    process_t *p = NULL;

    if (pid < 0)
        return -ECHILD;

repeat:
    while ((p = sched_lookup_finished(pid)) != NULL) {
        if (p->state == PROCESS_FINISHED) {
            int status = p->status;
            release_process(p);
            return status;
        } else {
            break;
        }
    }

    if (p == NULL) {
        context_switch(1);
        goto repeat;
    }

    return -ECHILD;
}

void process_wakeup_sleepers(uint32_t ticks, uint32_t subticks) {
    IRQ_OFF;
    ASSERT(_sleep_queue);
    if (_sleep_queue && _sleep_queue->length) {
        process_t *p = _sleep_queue->head->value;
        list_node_t *n;
        while (p && (p->end_tick < ticks ||
                     (p->end_tick == ticks && p->end_subtick <= subticks))) {
            p->end_tick = 0;
            p->end_subtick = 0;
            p->status = PROCESS_READY;

            sched_enqueue(p);

            n = list_pop_front(_sleep_queue);
            kfree(n);

            if (_sleep_queue->length) {
                ASSERT(_sleep_queue->head);
                p = _sleep_queue->head->value;
            } else {
                break;
            }
        }
    }
    IRQ_ON;
}

int sleep_on(list_t *queue) {
    list_push_front(queue, _current_process);

    context_switch(0);

    return 0;
}

int wakeup_from(list_t *queue) {
    list_node_t *n = list_pop_back(queue);

    if (!n)
        return 0;

    process_t *p = (process_t *)n->value;
    kfree(n);
    ASSERT(p);

    sched_enqueue(p);

    return 0;
}
