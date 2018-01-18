#include "hal/descriptor.h"
#include "hal/common.h"
#include "hal/isr.h"

#define IRQ_CHAIN_SIZE 16

#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_OFFSET 0x20
#define PIC1_DATA (PIC1 + 1)

#define PIC2 0xa0
#define PIC2_COMMAND PIC2

#define PCI_EOI 0x20

#define PIC_WAIT()            \
    do {                      \
        /* May be fragile */  \
        __asm__ __volatile__( \
            "jmp 1f\n\t"      \
            "1:\n\t"          \
            "    jmp 2f\n\t"  \
            "2:");            \
    } while (0)

static gdt_t _gdt[6];
static gdtr_t _gdtr;

static idt_t _idt[256];
static idtr_t _idtr;

extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);

extern void tss_flush();

tss_entry_t tss_entry;

void set_kernel_stack(uint32_t stack) { tss_entry.esp0 = stack; }

static void gdt_install() { __asm__ __volatile__("lgdt %0" ::"m"(_gdtr)); }

static void gdt_set_gate(int32_t i, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t grand) {
    _gdt[i].base_low = base & 0xffff;
    _gdt[i].base_mid = (base >> 16) & 0xff;
    _gdt[i].base_high = (base >> 24) & 0xff;
    _gdt[i].limit = limit & 0xffff;

    _gdt[i].flags = access;
    _gdt[i].grand = (limit >> 16) & 0x0f;
    _gdt[i].grand |= grand & 0xf0;
}

static void idt_set_gate(uint8_t i, uint32_t base, uint16_t selector,
                         uint8_t flags) {
    _idt[i].base_low = base & 0xFFFF;
    _idt[i].base_high = (base >> 16) & 0xFFFF;

    _idt[i].selector = selector;
    _idt[i].reserved = 0;
    _idt[i].flags = flags | 0x60;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = base + sizeof(tss_entry_t);

    gdt_set_gate(num, base, limit, 0xE9, 0x00);
    memset(&tss_entry, 0, sizeof(tss_entry_t));

    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;

    tss_entry.cs = 0x0b;
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs =
        0x13;
}

static void gdt_init() {
    _gdtr.limit = sizeof(gdt_t) * 6 - 1;
    _gdtr.base = (uint32_t)&_gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);  // kernel code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);  // kernel data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);  // user code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);  // user data segment

    write_tss(5, 0x10, 0);

    gdt_flush((uint32_t)&_gdtr);
    tss_flush();
}

static void idt_init() {
    int i;

    _idtr.limit = sizeof(idt_t) * 256 - 1;
    _idtr.base = (uint32_t)&_idt;

    memset((void *)&_idt, 0, sizeof(idt_t) * 256);

    // Remap the irq table.
    outb(0x20, 0x11);   PIC_WAIT();
    outb(0xA0, 0x11);   PIC_WAIT();
    outb(0x21, 0x20);   PIC_WAIT();
    outb(0xA1, 0x28);   PIC_WAIT();
    outb(0x21, 0x04);   PIC_WAIT();
    outb(0xA1, 0x02);   PIC_WAIT();
    outb(0x21, 0x01);   PIC_WAIT();
    outb(0xA1, 0x01);   PIC_WAIT();
    

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
    idt_set_gate(128, (uint32_t)isr128, 0x08, 0x8E);

    idt_flush((uint32_t)&_idtr);
}

void init_descriptor_tables() {
    gdt_init();
    idt_init();
}

void irq_ack(uint32_t irq_no) {
    if (irq_no > 8) {
        outb(PIC2_COMMAND, PCI_EOI);
    }

    outb(PIC1_COMMAND, PCI_EOI);
}
