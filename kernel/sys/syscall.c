#include "kernel/syscall.h"
#include "hal/isr.h"
#include "kernel/process.h"

DEFN_SYSCALL0(say, 0);

uint32_t nsyscalls = 1;

static void syscall_handler(registers_t *regs);

void syscalls_init() { register_interrupt_handler(0x80, syscall_handler); }

static void *syscalls[3] = {&say};

void syscall_handler(registers_t *regs) {
    if (regs->eax >= nsyscalls)
        return;

    void *location = syscalls[regs->eax];

    int ret;
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
    regs->eax = ret;
}
