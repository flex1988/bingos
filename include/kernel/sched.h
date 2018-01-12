#ifndef __SCHED_H__
#define __SCHED_H__
void sched_init();
void sched_enqueue(process_t *process);
process_t *sched_dequeue();
int sched_available();
void sleep_enqueue(process_t *process, uint32_t seconds, uint32_t subseconds);
#endif
