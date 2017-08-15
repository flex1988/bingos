#include "hal/common.h"
#include "kernel.h"

void outb(uint16_t port, uint8_t value) { asm volatile("outb %1, %0" : : "dN"(port), "a"(value)); }

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

int local_irq_enable() {
    unsigned long flags;

    asm volatile("pushf; sti; pop %0" : "=r"(flags));
    return (flags & (1 << 9)) ? 1 : 0;
}
