#include "hal/common.h"
#include "kernel.h"

void outb(uint16_t port, uint8_t value) { __asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(value)); }

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

uint16_t ins(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

void outs(uint16_t port, uint16_t data) { __asm__ __volatile__("outw %1, %0" ::"dN"(port), "a"(data)); }

void outl(uint16_t port, uint32_t data) { __asm__ __volatile__("outl %%eax, %%dx" ::"dN"(port), "a"(data)); }

void inl(uint16_t port) {
    uint32_t ret;
    __asm__ __volatile__("inl %%dx, %%eax" : "=a"(ret) : "dN"(port));
    return ret;
}

void insm(uint16_t port, uint8_t *data, uint32_t size) { __asm__ __volatile__("rep insw" : "+D"(data), "+c"(size) : "d"(port) : "memory"); }

void outsm(uint16_t port, uint8_t *data, uint32_t size) { __asm__ __volatile__("rep outsw" : "+S"(data), "+c"(size) : "d"(port)); }

void insl(uint16_t port, uint8_t *buf, uint32_t size) { __asm__ __volatile__("cld;rep;insl\n\t" ::"d"(port), "D"(buf), "c"(size)); }

int local_irq_enable() {
    unsigned long flags;

    __asm__ __volatile__("pushf; sti; pop %0" : "=r"(flags));
    return (flags & (1 << 9)) ? 1 : 0;
}
