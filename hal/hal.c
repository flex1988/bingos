#include "hal/cpu.h"

void disable() { asm volatile("cli"); }
void enable() { asm volatile("sti"); }

void hal_init() {
    disable();
    cpu_init();
    //enable();
}
