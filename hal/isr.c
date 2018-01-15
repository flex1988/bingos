#include "hal/isr.h"
#include "hal/common.h"
#include "kernel.h"

isr_t _interrupt_handlers[256];

/*
 * Software interrupt handler, call the exception handlers
 */
void isr_handler(registers_t *regs) {
    uint8_t int_no = regs->int_no;
    if (_interrupt_handlers[int_no]) {
        isr_t handler = _interrupt_handlers[int_no];
        handler(regs);
    } else {
        printk("unhandled interrupt: %d", int_no);
        PANIC("unhandled interrupt");
    }
}

void irq_handler(registers_t *regs) {
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (regs->int_no >= 40) {
        // Send reset signal to slave.
        outb(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    outb(0x20, 0x20);

    if (_interrupt_handlers[regs->int_no]) {
        isr_t handler = _interrupt_handlers[regs->int_no];

        handler(regs);

    }
}
void register_interrupt_handler(uint8_t n, isr_t handler) {
    _interrupt_handlers[n] = handler;
}
