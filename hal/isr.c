#include "hal/isr.h"
#include "hal/common.h"
#include "kernel.h"

#define ISR_COUNT 32

static irq_handler_t isr_routines[256] = {0};

void isrs_install_handler(uint8_t isrs, irq_handler_t handler) {
    isr_routines[isrs] = handler;
}

void isrs_uninstall_handler(uint8_t isrs) { isr_routines[isrs] = 0; }

static const char *exception_messages[32] = {"Division by zero",
                                             "Debug",
                                             "Non-maskable interrupt",
                                             "Breakpoint",
                                             "Detected overflow",
                                             "Out-of-bounds",
                                             "Invalid opcode",
                                             "No coprocessor",
                                             "Double fault",
                                             "Coprocessor segment overrun",
                                             "Bad TSS",
                                             "Segment not present",
                                             "Stack fault",
                                             "General protection fault",
                                             "Page fault",
                                             "Unknown interrupt",
                                             "Coprocessor fault",
                                             "Alignment check",
                                             "Machine check",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved",
                                             "Reserved"};

void fault_handler(registers_t *regs) {
    irq_handler_t handler = isr_routines[regs->int_no & 0x000000ff];

    if (handler) {
        handler(regs);
    } else {
        printk("Unhandled exception: [%d] %s", regs->int_no,
               exception_messages[regs->int_no]);
        PANIC(0);
    }
}
