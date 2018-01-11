#include <types.h>
#include "drivers/cmos.h"

#include "hal/common.h"
#include "hal/isr.h"
#include "kernel.h"
#include "kernel/process.h"
#include "kernel/vga.h"

#define SUBTICKS_PER_TICK 1000
#define RESYNC_TIME 1

#define PIT_A 0x40
#define PIT_B 0x41
#define PIT_C 0x42
#define PIT_CONTROL 0x43

#define PIT_MASK 0xff
#define PIT_SCALE 1193180
#define PIT_SET 0x34

uint32_t timer_ticks = 0;
uint32_t timer_subticks = 0;
int32_t timer_drift = 0;

static int behind = 0;

void timer_phase(int hz) {
    int divisor = PIT_SCALE / hz;
    outb(PIT_CONTROL, PIT_SET);
    outb(PIT_A, divisor & PIT_MASK);
    outb(PIT_A, (divisor >> 8) & PIT_MASK);
}

static void timer_callback(registers_t *regs) {
    if (++timer_subticks == SUBTICKS_PER_TICK) {
        timer_ticks++;
        timer_subticks = 0;

        if (timer_ticks % RESYNC_TIME == 0 ||
            (behind && timer_subticks++ == SUBTICKS_PER_TICK)) {
            uint32_t new_time = read_cmos();
            timer_drift = new_time - boot_time - timer_ticks;
            if (timer_drift > 0)
                behind = 1;
            else
                behind = 0;
        }
    }

    irq_ack(IRQ0);

    context_switch(1);
}

void timer_init() {
    register_interrupt_handler(IRQ0, &timer_callback);

    boot_time = read_cmos();

    timer_phase(SUBTICKS_PER_TICK);
}
