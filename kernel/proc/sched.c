#include "kernel.h"
#include "kernel/process.h"
#include "lib/list.h"

volatile process_t *_current_process = NULL;
volatile process_t *_prev_process = NULL;
volatile process_t *_next_process = NULL;
volatile process_t *_ready_queue = NULL;

volatile list_t *_process_queue;

void sched_init() { _process_queue = list_create(); }

void sched_enqueue(process_t *p) {
    /*process_t *t = (process_t *)_ready_queue;*/
    /*while (t->next) t = t->next;*/
    /*t->next = new;*/

    list_push_back(_process_queue, (void *)p);
}

process_t *sched_dequeue() {
    list_node_t *n = list_pop_front(_process_queue);

    if (!n)
        return NULL;

    process_t *p = (process_t *)n->value;

    kfree(n);

    return p;
}

int sched_available() { return _process_queue->head != 0; }
