#include "hal/isr.h"

#define SYNC_CLI() __asm__ __volatile__("cli")
#define SYNC_STI() __asm__ __volatile__("sti")

#define IRQ_CHAIN_SIZE 16
#define IRQ_CHAIN_DEPTH 4

static volatile int sync_depth = 0;

static irq_handler_chain_t irq_routines[IRQ_CHAIN_SIZE * IRQ_CHAIN_DEPTH] = {
    NULL};

void register_interrupt_handler(size_t irq, irq_handler_chain_t handler) {
    SYNC_CLI();
    for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
        if (irq_routines[i * IRQ_CHAIN_SIZE + irq])
            continue;
        irq_routines[i * IRQ_CHAIN_SIZE + irq] = handler;
        break;
    }
    SYNC_STI();
}

void interrupt_enable(void) {
    sync_depth = 0;
    SYNC_STI();
}

void interrupt_disable(void) {
    uint32_t flags;
    __asm__ __volatile__(
        "pushf\n\t"
        "pop %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(flags)
        :
        : "%eax");

    SYNC_CLI();

    if (flags & (1 << 9)) {
        sync_depth = 1;
    } else {
        sync_depth++;
    }
}

void interrupt_resume(void) {
    if (sync_depth == 0 || sync_depth == 1) {
        SYNC_STI();
    } else {
        sync_depth--;
    }
}

void irq_handler(registers_t *regs) {
    // Disable interrupts when handling
    interrupt_disable();

    if (regs->int_no <= 47 && regs->int_no >= 32) {
        for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
            irq_handler_chain_t handler =
                irq_routines[i * IRQ_CHAIN_SIZE + (regs->int_no - 32)];
            if (handler && handler(regs)) {
                goto done;
            }
        }
        irq_ack(regs->int_no - 32);
    }

done:
    interrupt_resume();
}
