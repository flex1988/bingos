#include <types.h>

#include "hal/isr.h"
#include "kernel.h"
#include "kernel/process.h"
#include "kernel/syscall.h"
#include "kernel/vga.h"

uint32_t nsyscalls = 13;

extern process_t* _current_process;

static void syscall_handler(registers_t* regs);

void syscalls_init() { register_interrupt_handler(0x80, syscall_handler); }

static void* syscalls[] = {
    sys_exit,     // 0
    sys_println,  // 1
    sys_open,     // 2
    sys_read,     // 3
    sys_write,    // 4
    sys_close,    // 5
    sys_printc,   // 6
    sys_execve,   // 7
    sys_fork,     // 8
    sys_getpid,   // 9
    sys_waitpid,  // 10
    sys_brk,      // 11
    sys_exit      // 12
};

void syscall_handler(registers_t* regs) {
    if (regs->eax >= nsyscalls)
        return;

    uint32_t location = syscalls[regs->eax];

    if (!location)
        return;

    _current_process->syscall_regs = regs;

    int ret;

    __asm__ __volatile__(
        "   push %1;    \
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

    regs->eax = ret;
}

// must not return ,switch process instead
int sys_exit(int ret) {
    process_exit(ret);

    while (1)
        ;
    return ret;
}

int sys_println(const char* msg) {
    printk(msg);
    return 0;
}

int sys_printc(char c) {
    printc(c);
    return 0;
}

int sys_write(int fd, const void* buf, size_t nbytes) { return 0; }
int sys_close(int fd) { return 0; }
int sys_gettimeofday() { return 0; }

int sys_execve(const char* filename, char* const argv[], char* const envp[]) {
    int argc = 0;

    while (argv[argc++])
        ;

    return sys_exec(filename, argc, argv);
}
