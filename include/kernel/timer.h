#ifndef __TIMER_H__
#define __TIMER_H__

#include <types.h>

extern uint32_t timer_ticks;
extern uint32_t timer_subticks;
extern int32_t timer_drift;

void timer_init();
void relative_time(uint32_t seconds, uint32_t subseconds, uint32_t *out_seconds, uint32_t *out_subseconds);
#endif
