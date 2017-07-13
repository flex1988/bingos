#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

//! i86 defines 256 possible interrupt handlers (0-255)
#define I86_MAX_INTERRUPTS 256

//! must be in the format 0D110, where D is descriptor type
#define I86_IDT_DESC_BIT16 0x06    // 00000110
#define I86_IDT_DESC_BIT32 0x0E    // 00001110
#define I86_IDT_DESC_RING1 0x40    // 01000000
#define I86_IDT_DESC_RING2 0x20    // 00100000
#define I86_IDT_DESC_RING3 0x60    // 01100000
#define I86_IDT_DESC_PRESENT 0x80  // 10000000

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
