#include <types.h>

#include "hal/isr.h"
#include "kernel.h"
#include "kernel/memlayout.h"
#include "kernel/process.h"
#include "kernel/syscall.h"
#include "kernel/vga.h"

static void syscall_handler(registers_t* regs);

static void* syscalls[] = {
    sys_exit,          // 0
    sys_println,       // 1
    sys_open,          // 2
    sys_read,          // 3
    sys_write,         // 4
    sys_close,         // 5
    sys_printc,        // 6
    sys_execve,        // 7
    sys_fork,          // 8
    sys_getpid,        // 9
    sys_waitpid,       // 10
    sys_brk,           // 11
    sys_stat,          // 12
    sys_readdir,       // 13
    sys_socketcall,    // 14
    sys_gettimeofday,  // 15
    sys_mkdir          // 16
};

uint32_t NR_syscalls = sizeof(syscalls) / sizeof(void*);

void syscalls_init() {
    printk("Init syscall table size: %d", NR_syscalls);
    isrs_install_handler(0x80, syscall_handler);
}

void syscall_handler(registers_t* regs) {
    if (regs->eax >= NR_syscalls)
        return;

    uint32_t location = syscalls[regs->eax];

    if (!location)
        return;

    CP->syscall_regs = regs;

    uint32_t ret;

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

    if (CP->syscall_regs == regs || (location != (uint32_t)sys_fork)) {
        regs->eax = ret;
    }
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

int sys_execve(const char* filename, char* const argv[], char* const envp[]) {
    int argc = 0;

    while (argv[argc]) argc++;
    return sys_exec(filename, argc, argv);
}
