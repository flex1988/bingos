#include <stdint.h>

#include "hal/idt.h"

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

static idt_descriptor_t _idt[I86_MAX_INTERRUPTS];

static idtr_t _idtr;

static void idt_install() { asm volatile("lidt (%0)" ::"m"(_idtr)); }

void default_handler() {
    for (;;)
        ;
}

void install_interrupt_routine(uint32_t i, uint16_t flags, uint16_t selector, IRQ_HANDLER irq) {
    uint64_t base = (uint64_t) & (*irq);

    _idt[i].base_low = base & 0xffff;
    _idt[i].base_high = (base >> 16) & 0xffff;
    _idt[i].reserved = 0;
    _idt[i].flags = flags;
    _idt[i].selector = selector;
}

void idt_init(uint16_t code_selector) {
    int i;

    _idtr.limit = sizeof(idt_descriptor_t) * I86_MAX_INTERRUPTS - 1;
    _idtr.base = (uint32_t)&_idt[0];

    memset((void*)&_idt[0], 0, sizeof(idt_descriptor_t) * I86_MAX_INTERRUPTS - 1);

    for (i = 0; i < I86_MAX_INTERRUPTS; i++)
        install_interrupt_routine(i, I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32, code_selector, (IRQ_HANDLER)default_handler);

    idt_install();
}
