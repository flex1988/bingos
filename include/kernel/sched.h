#ifndef __SCHED_H__
#define __SCHED_H__
void sched_init();
void sched_enqueue(process_t *process, int priority);
process_t *sched_dequeue();
int sched_available();
#endif
