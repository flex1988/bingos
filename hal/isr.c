#include "hal/isr.h"

/*
 * Software interrupt handler, call the exception handlers
 */
void isr_handler(registers_t regs) {
    irq_hook_t *hook;
    boolean_t processed = FALSE;
    uint8_t int_no;

    /* Avoid the problem caused by the signed interrupt number if it is
     * more than 0x80
     */
    int_no = (uint8_t)regs.int_no;

    /* Call each handler on the ISR hook chain */
    hook = _irq_chains[int_no].head;
    while (hook) {
        isr_t handler = hook->handler;
        if (handler) {
            handler(&regs);
            processed = TRUE;
        }
        hook = hook->next;
    }

    if (!processed) {
        kprintf("ISR %d not handled\n", int_no);
        for (;;)
            ;
    }
}
