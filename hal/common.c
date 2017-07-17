#include "hal/common.h"
#include "kernel.h"

void outb(uint16_t port, uint8_t value) { asm volatile("outb %1, %0" : : "dN"(port), "a"(value)); }
