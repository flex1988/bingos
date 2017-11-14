#include "hal/isr.h"
#include "kernel.h"
#include "kernel/process.h"
#include "kernel/vga.h"

uint32_t nsyscalls = 6;

extern process_t *_current_process;

static void syscall_handler(registers_t *regs);

int sys_exit(int ret);

void syscalls_init() { register_interrupt_handler(0x80, syscall_handler); }

static void *syscalls[6] = {&println, &sys_exec, NULL, &sys_fork, NULL, &sys_exit};

void syscall_handler(registers_t *regs) {
    if (regs->eax >= nsyscalls)
        return;

    void *location = syscalls[regs->eax];
    printk("syscall %d", regs->eax);

    int ret;
    _current_process->syscall_regs = regs;

    asm volatile(
        "\
            push %1;    \
            push %2;    \
            push %3;    \
            push %4;    \
            push %5;    \
            call *%6;   \
            pop %%ebx;  \
            pop %%ebx;  \
            pop %%ebx;  \
            pop %%ebx;  \
            pop %%ebx;  \
        "
        : "=a"(ret)
        : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(location));

    regs = _current_process->syscall_regs;
    regs->eax = ret;
}

// must not return ,switch process instead
int sys_exit(int ret) {
    printk("exit");
    process_exit(ret);

    while (1)
        ;
    return ret;
}
