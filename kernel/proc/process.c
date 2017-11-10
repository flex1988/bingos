#include "kernel/process.h"
#include "fs/fs.h"
#include "kernel.h"
#include "kernel/elf.h"
#include "kernel/kheap.h"
#include "kernel/mmu.h"
#include "kernel/sched.h"

/*volatile process_t *_current_process;*/
/*volatile process_t *_ready_queue;*/

extern page_dir_t *_kernel_pd;
extern page_dir_t *_current_pd;
extern void page_map(page_t *, int, int);
extern uint32_t _initial_esp;
extern uint32_t read_eip();

extern process_t *_current_process;
extern process_t *_ready_queue;
process_t *_init_process;

uint32_t _next_pid = 1;

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

process_t *process_create(process_t *parent) {
    process_t *p = kmalloc_i(sizeof(process_t), 0, 0);
    p->id = _next_pid++;
    p->uid = 500;
    p->gid = 500;
    p->next = 0;
    memcpy(p->name, "init", sizeof("init"));

    p->esp = 0;
    p->ebp = 0;
    p->eip = 0;

    p->state = PROCESS_READY;

    /*if (parent) {*/
        p->kstack = kmalloc_i(sizeof(KSTACK_SIZE), 1, 0);
        memset((void *)p->kstack, 0, KSTACK_SIZE);

        p->ustack = parent->ustack;
    /*} else {*/
        /*p->ustack = 0;*/
    /*}*/

    p->status = 0;
}

void process_exit(int ret) {
    _current_process->status = ret;
    _current_process->state = PROCESS_FINISHED;

    process_t *prev;
    process_t *iter = _ready_queue;
    while (iter != _current_process) {
        prev = iter;
        iter = iter->next;
    }

    prev->next = iter->next;

    context_switch();
}

void switch_to_user_mode(uint32_t location, uint32_t ustack) {
    set_kernel_stack(_current_process->kstack + KSTACK_SIZE);
    do_switch_to_user_mode();

    /*asm volatile(*/
        /*"cli\n"*/
        /*"mov %1, %%esp\n"*/
        /*"mov $0x23, %%ax\n" [> Segment selector <]*/
        /*"mov %%ax, %%ds\n"*/
        /*"mov %%ax, %%es\n"*/
        /*"mov %%ax, %%fs\n"*/
        /*"mov %%ax, %%gs\n"*/
        /*"mov %%esp, %%eax\n" [> Move stack to EAX <]*/
        /*"pushl $0x23\n"      [> Segment selector again <]*/
        /*"pushl %%eax\n"*/
        /*"pushf\n"     [> Push flags <]*/
        /*"pop %%eax\n" [> Enable the interrupt flag <]*/
        /*"orl $0x200, %%eax\n"*/
        /*"push %%eax\n"*/
        /*"pushl $0x1B\n"*/
        /*"pushl %0\n" [> Push the entry point <]*/
        /*"iret\n" ::"m"(location),*/
        /*"r"(ustack)*/
        /*: "%ax", "%esp", "%eax");*/
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

    for (i = new_stack_start - size; i < new_stack_start + 0x1000; i += 0x1000) {
        page_t *p = get_page(i, 1, _current_pd);
        page_map(p, 0, 1);
    }

    // flush the TLB by reading and writing the page directory address again
    uint32_t pd_addr;
    asm volatile("mov %%cr3, %0" : "=r"(pd_addr));
    asm volatile("mov %0, %%cr3" ::"r"(pd_addr));

    uint32_t old_stack_pointer;
    asm volatile("mov %%esp, %0" : "=r"(old_stack_pointer));
    uint32_t old_base_pointer;
    asm volatile("mov %%ebp, %0" : "=r"(old_base_pointer));

    uint32_t offset = (uint32_t)new_stack_start - _initial_esp;

    uint32_t new_stack_pointer = old_stack_pointer + offset;
    uint32_t new_base_pointer = old_base_pointer + offset;

    memcpy((void *)new_stack_pointer, (void *)old_stack_pointer, _initial_esp - old_stack_pointer);

    for (i = (uint32_t)new_stack_start; i > (uint32_t)new_stack_start - size; i -= 4) {
        uint32_t tmp = *(uint32_t *)i;
        if ((old_stack_pointer < tmp) && (tmp < _initial_esp)) {
            tmp = tmp + offset;
            uint32_t *tmp2 = (uint32_t *)i;
            *tmp2 = tmp;
        }
    }

    asm volatile("mov %0, %%esp" ::"r"(new_stack_pointer));
    asm volatile("mov %0, %%ebp" ::"r"(new_base_pointer));
    printk("relocate esp 0x%x, ebp 0x%x", new_stack_pointer, new_base_pointer);
}

void process_init() {
    asm volatile("cli");

    // relocate the stack to 0xe0000000
    move_stack(0xe0000000, 0x2000);

    process_t *init = process_create(0);

    init->pd = _current_pd;
    init->kstack = 0xe0000000;
    init->state = PROCESS_RUNING;

    _current_process = _ready_queue = _init_process = init;

    asm volatile("sti");
}

int say() {
    printk("hello world!");
    return 0;
}

/*int sys_fork() {*/
/*asm volatile("cli");*/

/*ASSERT(_init_process);*/
/*process_t *parent = (process_t *)_init_process;*/
/*[>process_t *parent = (process_t *)_current_process;<]*/

/*process_t *new = process_create(parent);*/

/*new->pd = page_dir_clone(parent->pd);*/

/*sched_enqueue(new);*/

/*uint32_t eip = read_eip();*/

/*if (_current_process == parent) {*/
/*// parent*/
/*uint32_t esp;*/
/*asm volatile("mov %%esp, %0" : "=r"(esp));*/
/*uint32_t ebp;*/
/*asm volatile("mov %%ebp, %0" : "=r"(ebp));*/
/*uint32_t offset;*/

/*new->esp = esp;*/
/*new->ebp = ebp;*/
/*new->eip = eip;*/

/*if (parent->kstack > new->kstack) {*/
/*offset = parent->kstack - new->kstack;*/
/*new->esp = esp - offset;*/
/*new->ebp = ebp - offset;*/
/*} else {*/
/*offset = new->kstack - parent->kstack;*/
/*new->esp = esp + offset;*/
/*new->ebp = ebp + offset;*/
/*}*/

/*new->eip = eip;*/

/*memcpy(new->kstack - KSTACK_SIZE, parent->kstack - KSTACK_SIZE, KSTACK_SIZE);*/
/*relocate_stack(new->kstack, parent->kstack, KSTACK_SIZE);*/

/*offset = parent->kstack - (uint32_t)parent->syscall_regs;*/
/*new->syscall_regs = (registers_t *)(new->kstack - offset);*/

/*asm volatile("sti");*/
/*return new->id;*/
/*} else {*/
/*// child*/
/*return 0;*/
/*}*/
/*}*/

int sys_fork() {
    asm volatile("cli");

    process_t *parent = (process_t *)_current_process;

    process_t *new = process_create(parent);
    new->pd = page_dir_clone(_current_pd);

    sched_enqueue(new);

    uint32_t eip = read_eip();

    if (_current_process == parent) {
        // parent
        uint32_t esp;
        asm volatile("mov %%esp, %0" : "=r"(esp));
        uint32_t ebp;
        asm volatile("mov %%ebp, %0" : "=r"(ebp));
        uint32_t offset;

        new->esp = esp;
        new->ebp = ebp;
        new->eip = eip;

        /*if (parent->kstack > new->kstack) {*/
            /*offset = parent->kstack - new->kstack;*/
            /*new->esp = esp - offset;*/
            /*new->ebp = ebp - offset;*/
        /*} else {*/
            /*offset = new->kstack - parent->kstack;*/
            /*new->esp = esp + offset;*/
            /*new->ebp = ebp + offset;*/
        /*}*/

        /*new->eip = eip;*/

        /*memcpy(new->kstack - KSTACK_SIZE, parent->kstack - KSTACK_SIZE, KSTACK_SIZE);*/
        /*relocate_stack(new->kstack, parent->kstack, KSTACK_SIZE);*/

        /*offset = parent->kstack - (uint32_t)parent->syscall_regs;*/
        /*new->syscall_regs = (registers_t *)(new->kstack - offset);*/

        asm volatile("sti");
        return new->id;
    } else {
        // child
        return 0;
    }
}

void context_switch() {
    if (!_current_process)
        return;

    if (_current_process->interrupt) {
        _current_process->interrupt = 0;
        return;
    }

    uint32_t esp, ebp, eip;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    asm volatile("mov %%ebp, %0" : "=r"(ebp));

    eip = read_eip();
    // hack magic
    if (eip == 0x12345) {
        return;
    }

    _current_process->eip = eip;
    _current_process->esp = esp;
    _current_process->ebp = ebp;

    _current_process->interrupt = 1;

    _current_process = _current_process->next;
    if (!_current_process) {
        _current_process = _ready_queue;
        ASSERT(_current_process);
    }

    eip = _current_process->eip;
    esp = _current_process->esp;
    ebp = _current_process->ebp;

    _current_pd = _current_process->pd;

    set_kernel_stack(_current_process->kstack + KSTACK_SIZE);

    do_switch_task(
	    _current_process->eip,
	    _current_process->esp,
	    _current_process->ebp,
	    _current_pd->physical,
	    _current_process->interrupt
	);

    /*asm volatile(*/
        /*"         \*/
      /*cli;                 \*/
      /*mov %0, %%ecx;       \*/
      /*mov %1, %%esp;       \*/
      /*mov %2, %%ebp;       \*/
      /*mov %3, %%cr3;       \*/
      /*mov $0x12345, %%eax; \*/
      /*sti;                 \*/
      /*jmp *%%ecx           " ::"r"(eip),*/
        /*"r"(esp), "r"(ebp), "r"(_current_pd->physical)*/
        /*: "%ecx", "%esp", "%eax");*/
}

int getpid() { return _current_process->id; }

int sys_exec(char *path, int argc, char **argv) {
    int ret = -1;
    vfs_node_t *n;
    elf32_ehdr *ehdr;

    n = vfs_lookup(path, 0);

    ASSERT(n);

    ptr_t virt;
    ptr_t entry;
    page_t *page;

    for (virt = 0x30000000; virt < (0x30000000 + n->length); virt += PAGE_SIZE) {
        page = get_page(virt, 1, _current_process->pd);
        ASSERT(page);

        page_map(page, 0, 1);
    }

    ehdr = (elf32_ehdr *)0x30000000;

    ret = vfs_read(n, 0, n->length, (uint8_t *)ehdr);
    ASSERT(ret >= sizeof(elf32_ehdr));

    if (!elf_ehdr_check(ehdr)) {
        printk("invalid elf header");
        return -1;
    }

    if (!elf_load_sections(ehdr)) {
        printk("load elf sections error");
        return -1;
    }

    entry = ehdr->e_entry;

    // free user mode stack and file
    for (virt = 0x30000000; virt < (0x30000000 + n->length); virt += PAGE_SIZE) {
        page = get_page(virt, 0, _current_process->pd);
        ASSERT(page);

        page_unmap(page);
    }

    for (virt = USTACK_BOTTOM; virt <= (USTACK_BOTTOM + USTACK_SIZE); virt += PAGE_SIZE) {
        page = get_page(virt, 1, _current_process->pd);
        ASSERT(page);

        page_map(page, 0, 1);
    }

    _current_process->ustack = USTACK_BOTTOM + USTACK_SIZE;

    switch_to_user_mode(entry, USTACK_BOTTOM + USTACK_SIZE);

    return -1;
}
