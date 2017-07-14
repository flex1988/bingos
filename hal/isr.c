#include "kernel.h"
#include "hal/isr.h"

static irq_hook_t _irqs[256];

/*
 * Software interrupt handler, call the exception handlers
 */
void isr_handler(registers_t regs) {
    irq_hook_t *hook;
    bool_t processed = FALSE;
    uint8_t no;

    /* Avoid the problem caused by the signed interrupt number if it is
     * more than 0x80
     */
    no = (uint8_t)regs.int_no;

    /* Call each handler on the ISR hook chain */
    hook = &_irqs[no];
    isr_t handler = hook->handler;
    if (handler) {
        handler(&regs);
        processed = TRUE;
    }

    if (!processed) {
        printk("ISR %d not handled\n", no);
        for (;;)
            ;
    }
}

void irq_handler(registers_t regs) {
    uint8_t no;
    irq_hook_t *hook;
    bool_t processed = FALSE;

    no = (uint8_t)regs.int_no;

    hook = &_irqs[no];
    isr_t handler = hook->handler;
    if (handler) {
        handler(&regs);
        processed = TRUE;
    }

    if (!processed) {
        printk("IRQ %d not handled", no);
    }
}

void register_irq_handler(uint8_t irq, irq_hook_t *hook, isr_t handler) {
    irq_hook_t *line;

    line = &_irqs[irq];

    hook->handler = handler;
    hook->irq = irq;
    *line = *hook;
}

void divide_by_zero_fault(registers_t *regs) { PANIC("Divide by zero"); }

void single_step_fault(registers_t *regs) { PANIC("single step step"); }

void nmi_trap(registers_t *regs) { PANIC("NMI trap"); }

void breakpoint_trap(registers_t *regs) { PANIC("breakpoint trap"); }

void overflow_trap(registers_t *regs) { PANIC("overflow trap"); }
void bounds_check_fault(registers_t *regs) { PANIC("bounds check fault"); }
void invalid_opcode_fault(registers_t *regs) { PANIC("invliad opcode fault"); }
void no_device_fault(registers_t *regs) { PANIC("device not found"); }
void double_fault_abort(registers_t *regs) { PANIC("double fault"); }
void invalid_tss_fault(registers_t *regs) { PANIC("invalid tss"); }
void no_segment_fault(registers_t *regs) { PANIC("invalid segment"); }
void stack_fault(registers_t *regs) { PANIC("stack fault"); }
void general_protection_fault(registers_t *regs) { PANIC("general protection fault"); }
void fpu_fault(registers_t *regs) { PANIC("general protection fault"); }
void alignment_check_fault(registers_t *regs) { PANIC("alignment fault"); }
void machine_check_abort(registers_t *regs) { PANIC("machine check fault"); }
void simd_fpu_fault(registers_t *regs) { PANIC("SIMD FPU fault"); }

void irqs_init() {
    int i;
    for (i = 0; i < 256; i++) {
        memset(&_irqs[i], 0, sizeof(irq_hook_t));
    }

    register_irq_handler(0, &_irqs[0], divide_by_zero_fault);
    register_irq_handler(1, &_irqs[1], single_step_fault);
    register_irq_handler(2, &_irqs[2], nmi_trap);
    register_irq_handler(3, &_irqs[3], breakpoint_trap);
    register_irq_handler(4, &_irqs[4], overflow_trap);
    register_irq_handler(5, &_irqs[5], bounds_check_fault);
    register_irq_handler(6, &_irqs[6], invalid_opcode_fault);
    register_irq_handler(7, &_irqs[7], no_device_fault);
    register_irq_handler(8, &_irqs[8], double_fault_abort);
    register_irq_handler(10, &_irqs[9], invalid_tss_fault);
    register_irq_handler(11, &_irqs[10], no_segment_fault);
    register_irq_handler(12, &_irqs[11], stack_fault);
    register_irq_handler(13, &_irqs[12], general_protection_fault);
    register_irq_handler(16, &_irqs[13], fpu_fault);
    register_irq_handler(17, &_irqs[14], alignment_check_fault);
    register_irq_handler(18, &_irqs[15], machine_check_abort);
    register_irq_handler(19, &_irqs[16], simd_fpu_fault);

    printk("IRQ dispatch table initialized");
}
