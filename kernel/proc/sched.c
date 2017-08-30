#include "kernel.h"
#include "kernel/process.h"

volatile process_t *_current_process = NULL;
volatile process_t *_prev_process = NULL;
volatile process_t *_next_process = NULL;
volatile process_t *_ready_queue = NULL;

void sched_enqueue(process_t *new) {
    process_t *t = (process_t *)_ready_queue;
    while (t->next) t = t->next;
    t->next = new;
}
