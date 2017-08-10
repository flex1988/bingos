#include "proc/task.h"
#include "kernel.h"
#include "mm/kheap.h"
#include "mm/mmu.h"

volatile task_t *_current_task;
volatile task_t *_ready_queue;

extern page_dir_t *_kernel_pd;
extern page_dir_t *_current_pd;
extern void page_map(page_t *, int, int);
extern uint32_t _initial_esp;
extern uint32_t read_eip();

uint32_t _next_pid = 1;

void switch_to_user_mode() {
    set_kernel_stack(_current_task->kstack + KERNEL_STACK_SIZE);

    asm volatile("\
            cli;    \
            mov $0x23, %ax; \
            mov %ax, %ds;   \
            mov %ax, %es;   \
            mov %ax, %fs;   \
            mov %ax, %gs;   \
            \
            mov %esp, %eax; \
            pushl $0x23;    \
            pushl %esp;     \
            pushf;          \
            pushl $0x1b;    \
            push $1f;   \
            iret;   \
            1: ");
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

void task_init() {
    asm volatile("cli");

    // relocate the stack to 0xe0000000
    move_stack(0xe0000000, 0x2000);

    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    task->id = _next_pid++;
    task->esp = task->ebp = 0;
    task->eip = 0;
    task->pd = _current_pd;
    task->next = 0;
    task->kstack = kmalloc_i(KERNEL_STACK_SIZE, 1, 0);
    _current_task = _ready_queue = task;
    asm volatile("sti");
}

int say() {
    printk("hello world!");
    return 0;
}

int fork() {
    asm volatile("cli");

    task_t *parent = (task_t *)_current_task;

    page_dir_t *pd = page_dir_clone(_current_pd);

    task_t *new = (task_t *)kmalloc(sizeof(task_t));
    new->id = _next_pid++;
    new->esp = new->ebp = 0;
    new->eip = 0;
    new->pd = pd;
    new->next = 0;
    _current_task->kstack = kmalloc_i(KERNEL_STACK_SIZE, 1, 0);

    task_t *t = (task_t *)_ready_queue;
    while (t->next) t = t->next;
    t->next = new;

    uint32_t eip = read_eip();

    if (_current_task == parent) {
        // parent
        uint32_t esp;
        asm volatile("mov %%esp, %0" : "=r"(esp));
        uint32_t ebp;
        asm volatile("mov %%ebp, %0" : "=r"(ebp));

        new->esp = esp;
        new->ebp = ebp;
        new->eip = eip;

        asm volatile("sti");
        return new->id;
    } else {
        // child
        return 0;
    }
}

void task_switch() {
    if (!_current_task) {
        printk("_current_task is null");
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

    /*ASSERT(_current_task->eip != eip);*/
    _current_task->eip = eip;
    _current_task->esp = esp;
    _current_task->ebp = ebp;

    _current_task = _current_task->next;

    if (!_current_task) {
        _current_task = _ready_queue;
        ASSERT(_current_task);
    }

    eip = _current_task->eip;
    esp = _current_task->esp;
    ebp = _current_task->ebp;

    _current_pd = _current_task->pd;

    set_kernel_stack(_current_task->kstack + KERNEL_STACK_SIZE);

    asm volatile(
        "         \
      cli;                 \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%ebp;       \
      mov %3, %%cr3;       \
      mov $0x12345, %%eax; \
      sti;                 \
      jmp *%%ecx           " ::"r"(eip),
        "r"(esp), "r"(ebp), "r"(_current_pd->physical)
        : "%ecx", "%esp", "%eax");
}

int getpid() { return _current_task->id; }
