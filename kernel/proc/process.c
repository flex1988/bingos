#include "kernel/process.h"
#include "fs/fs.h"
#include "kernel.h"
#include "kernel/elf.h"
#include "kernel/kheap.h"
#include "kernel/mmu.h"
#include "kernel/sched.h"

#include <errno.h>

extern page_dir_t *_kernel_pd;
extern page_dir_t *_current_pd;
extern void page_map(page_t *, int, int);
extern uint32_t _initial_esp;
extern uint32_t read_eip();

process_t *_current_process;

process_t *_init_process = NULL;

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

int dup_mmap(process_t *p, process_t *parent) {
    vm_area_t *i, **ptr, *tmp;

    p->mmap = NULL;
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
    process_t *p = (process_t *)kmalloc_i(sizeof(process_t), 0, 0);
    p->id = _next_pid++;
    p->uid = 500;
    p->gid = 500;
    p->next = 0;
    memcpy(p->name, "[default]", sizeof("[default]"));

    p->esp = 0;
    p->ebp = 0;
    p->eip = 0;

    p->state = PROCESS_READY;

    if (parent) {
        p->kstack = kmalloc_i(sizeof(KSTACK_SIZE), 1, 0) + KSTACK_SIZE;
        memset((void *)p->kstack - KSTACK_SIZE, 0, KSTACK_SIZE);

        p->ustack = parent->ustack;

        dup_mmap(p, parent);
    } else {
        p->ustack = 0;
        p->mmap = 0;
    }

    p->status = 0;
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

void switch_to_user_mode(uint32_t location, uint32_t ustack) {
    IRQ_OFF;
    set_kernel_stack(_current_process->kstack);
    __asm__ __volatile__(
        "mov %1, %%esp\n"
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%esp, %%eax\n"
        "pushl $0x23\n"
        "pushl %%eax\n"
        "pushf\n"
        "pop %%eax\n"
        "orl $0x200, %%eax\n"
        "pushl %%eax\n"
        "pushl $0x1B\n"
        "pushl %0\n"
        "iret\n" ::"m"(location),
        "r"(ustack)
        : "%ax", "%esp", "%eax");
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
    __asm__ __volatile__("mov %%cr3, %0" : "=r"(pd_addr));
    __asm__ __volatile__("mov %0, %%cr3" ::"r"(pd_addr));

    uint32_t old_stack_pointer;
    __asm__ __volatile__("mov %%esp, %0" : "=r"(old_stack_pointer));
    uint32_t old_base_pointer;
    __asm__ __volatile__("mov %%ebp, %0" : "=r"(old_base_pointer));

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

    __asm__ __volatile__("mov %0, %%esp" ::"r"(new_stack_pointer));
    __asm__ __volatile__("mov %0, %%ebp" ::"r"(new_base_pointer));
    printk("relocate esp 0x%x, ebp 0x%x", new_stack_pointer, new_base_pointer);
}

void process_init() {
    IRQ_OFF;

    // relocate the stack to 0xe0000000
    move_stack(0xe0000000, 0x2000);

    sched_init();

    process_t *init = process_create(0);

    init->pd = _current_pd;
    init->kstack = 0xe0000000;
    init->state = PROCESS_RUNING;

    _current_process = _init_process = init;

    IRQ_ON;
}

int sys_fork() {
    IRQ_OFF;
    process_t *parent = (process_t *)_current_process;
    page_dir_t *ppd = page_dir_clone(parent->pd);

    process_t *new = process_create(parent);

    new->pd = ppd;

    uint32_t eip = read_eip();

    if (_current_process == parent) {
        // parent
        uint32_t esp;
        __asm__ __volatile__("mov %%esp, %0" : "=r"(esp));
        uint32_t ebp;
        __asm__ __volatile__("mov %%ebp, %0" : "=r"(ebp));

        if (parent->kstack > new->kstack) {
            new->esp = esp - (parent->kstack - new->kstack);
            new->ebp = ebp - (parent->kstack - new->kstack);
        } else {
            new->esp = esp + (new->kstack - parent->kstack);
            new->ebp = ebp + (new->kstack - parent->kstack);
        }

        memcpy((void *)new->kstack - KSTACK_SIZE, (void *)parent->kstack - KSTACK_SIZE, KSTACK_SIZE);

        new->eip = eip;

        sched_enqueue(new, 0);

        IRQ_ON;

        return new->id;
    } else {
        // child
        return 0;
    }
}

void context_switch() {
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

    ASSERT(_current_process);

    _current_process->eip = eip;
    _current_process->esp = esp;
    _current_process->ebp = ebp;

    sched_enqueue(_current_process, 0);

    switch_to_next();
}

void switch_to_next() {
    uint32_t esp, ebp, eip;

    _current_process = sched_dequeue();

    ASSERT(_current_process);

    eip = _current_process->eip;
    esp = _current_process->esp;
    ebp = _current_process->ebp;

    set_kernel_stack(_current_process->kstack);

    _current_pd = _current_process->pd;

    __asm__ __volatile__(
        "mov %0, %%ecx;       \
         mov %1, %%esp;       \
         mov %2, %%ebp;       \
         mov %3, %%cr3;       \
         mov $0x12345678, %%eax; \
         sti;                 \
         jmp *%%ecx" ::"r"(eip),
        "r"(esp), "r"(ebp), "r"(_current_pd->physical)
        : "%ecx", "%esp", "%eax");
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

    ret = do_mmap(USTACK_BOTTOM, n->length);
    ASSERT(!ret);

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

    ret = do_munmap(USTACK_BOTTOM, n->length);
    ASSERT(!ret);

    ret = do_mmap(USTACK_BOTTOM, USTACK_SIZE);
    ASSERT(!ret);

    _current_process->ustack = USTACK_BOTTOM + USTACK_SIZE;

    switch_to_user_mode(entry, _current_process->ustack);

    ASSERT(0);

    return ret;
}

int sys_getpid() {
    if (!_current_process)
        return 0;
    return _current_process->id;
}

int sys_waitpid(int pid) {
    process_t *p;

    if (pid < 0)
        return -ECHILD;

repeat:
    IRQ_ON;
    while ((p = sched_lookup_finished(pid)) != NULL) {
        if (p->state == PROCESS_FINISHED) {
            IRQ_OFF;
            // free process
            return p->status;
        } else {
            break;
        }
    }

    if (p == NULL) {
        goto repeat;
    }

    IRQ_OFF;
    return -ECHILD;
}
