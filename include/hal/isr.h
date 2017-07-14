#ifndef __ISR_H__
#define __ISR_H__

#include <types.h>

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

/*
 * Note that gs, fs segment register was reserved for other
 * purpose, so we don't put it here
 */
typedef struct {
    uint32_t es;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no;
    uint32_t err_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t ss;
} registers_t;

typedef void (*isr_t)(registers_t *);

typedef struct {
    isr_t handler;
    uint8_t irq;
} irq_hook_t;

extern void init_irqs();
extern void dump_registers(registers_t *regs);
extern void register_irq_handler(uint8_t irq, irq_hook_t *hook, isr_t handler);
extern void unregister_irq_handler(irq_hook_t *hook);

#endif /* __ISR_H__ */
