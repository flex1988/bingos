#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

typedef void (*IRQ_HANDLER)(void);

typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

extern idt_t* i86_get_ir(uint32_t i);

extern void install_interrupt_routine(uint32_t i, uint16_t flags, uint16_t sel, IRQ_HANDLER);

#endif
