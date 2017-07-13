#include "hal/gdt.h"

static gdt_t _gdt[GDT_LENGTH];
static gdtr_t _gdtr;

static void gdt_install() { asm volatile("lgdt %0" ::"m"(_gdtr)); }

static void gdt_set_gate(uint32_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t grand) {
    memset((void *)&_gdt[i], 0, sizeof(gdt_t));

    _gdt[i].base_low = base & 0xffff;
    _gdt[i].base_mid = (base >> 16) & 0xff;
    _gdt[i].base_high = (base >> 24) & 0xff;
    _gdt[i].limit = limit & 0xffff;

    _gdt[i].flags = access;
    _gdt[i].grand = (limit >> 16) & 0x0f;
    _gdt[i].grand |= grand & 0xf0;
}

void gdt_init() {
    _gdtr.limit = sizeof(gdt_t) * GDT_LENGTH - 1;
    _gdtr.base = (uint32_t)&_gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);  // kernel code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);  // kernel data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);  // user code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);  // user data segment

    gdt_install();
}
