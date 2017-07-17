#include "hal/descriptor.h"
#include "hal/isr.h"
#include "hal/common.h"

static gdt_t _gdt[5];
static gdtr_t _gdtr;

static idt_t _idt[256];
static idtr_t _idtr;

extern isr_t _interrupt_handlers[];

extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);

static void gdt_install() { asm volatile("lgdt %0" ::"m"(_gdtr)); }

static void gdt_set_gate(int32_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t grand) {
    _gdt[i].base_low = base & 0xffff;
    _gdt[i].base_mid = (base >> 16) & 0xff;
    _gdt[i].base_high = (base >> 24) & 0xff;
    _gdt[i].limit = limit & 0xffff;

    _gdt[i].flags = access;
    _gdt[i].grand = (limit >> 16) & 0x0f;
    _gdt[i].grand |= grand & 0xf0;
}

static void idt_set_gate(uint8_t i, uint32_t base, uint16_t selector, uint8_t flags) {
    _idt[i].base_low = base & 0xFFFF;
    _idt[i].base_high = (base >> 16) & 0xFFFF;

    _idt[i].selector = selector;
    _idt[i].reserved = 0;
    _idt[i].flags = flags /* | 0x60 */;
}

static void gdt_init() {
    _gdtr.limit = sizeof(gdt_t) * 5 - 1;
    _gdtr.base = (uint32_t)&_gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);  // kernel code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);  // kernel data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);  // user code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);  // user data segment

    gdt_flush((uint32_t)&_gdtr);
}

static void idt_init() {
    int i;

    _idtr.limit = sizeof(idt_t) * 256 - 1;
    _idtr.base = (uint32_t)&_idt;

    memset((void *)&_idt, 0, sizeof(idt_t) * 256);

    // Remap the irq table.
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    /* set default interrupt service routine */
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    /* The following are all IRQs */
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    idt_flush((uint32_t)&_idtr);

    memset(&_interrupt_handlers, 0, sizeof(isr_t) * 256);
}

void init_descriptor_tables() {
    gdt_init();
    idt_init();
}
