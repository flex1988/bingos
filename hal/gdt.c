#include "hal/gdt.h"

typedef struct {
    uint16_t m_limit;
    uint32_t m_base;
} __attribute__((packed)) gdtr_t;

static gdt_descriptor_t _gdt[MAX_DESCRIPTORS];
static gdtr_t _gdtr;

static void gdt_install() { asm volatile("lgdt (%0)" ::"m"(_gdtr)); }

static void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand) {
    memset((void *)&_gdt[i], 0, sizeof(gdt_descriptor_t));

    _gdt[i].base_low = base & 0xffff;
    _gdt[i].base_mid = (base >> 16) & 0xff;
    _gdt[i].base_high = (base >> 24) & 0xff;
    _gdt[i].limit = limit & 0xffff;

    _gdt[i].flags = access;
    _gdt[i].grand = (limit >> 16) & 0x0f;
    _gdt[i].grand |= grand & 0xf0;
}

void gdt_init() {
    _gdtr.m_limit = (sizeof(gdtr_t) * 3) - 1;
    _gdtr.m_base = (uint32_t)&_gdt[0];

    gdt_set_descriptor(0, 0, 0, 0, 0);

    //! set default code descriptor
    gdt_set_descriptor(1, 0, 0xffffffff, I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
                       I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);
    //! set default data descriptor
    gdt_set_descriptor(2, 0, 0xffffffff, I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
                       I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

    gdt_install();
}
