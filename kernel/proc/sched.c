#include "kernel.h"
#include "kernel/process.h"
#include "lib/list.h"

volatile list_t *_process_queue;
volatile list_t *_finished_queue;

void sched_init() {
    _process_queue = list_create();
    _finished_queue = list_create();
}

void sched_enqueue(process_t *p) {
    ASSERT(p);
    list_push_front(_process_queue, (void *)p);
}

process_t *sched_dequeue() {
    list_node_t *n = list_pop_back(_process_queue);

    if (!n)
        return NULL;

    process_t *p = (process_t *)n->value;
    kfree(n);
    ASSERT(p);

    return p;
}

void sched_enqueue_finished(process_t *p) {
    ASSERT(p);
    list_push_back(_finished_queue, (void *)p);
}

process_t *sched_lookup_finished(int pid) {
    ASSERT(_finished_queue);

    list_node_t *n = _finished_queue->head;
    for (; n != NULL; n = n->next) {
        process_t *p = (process_t *)n->value;
        ASSERT(p);
        if (p->id == pid)
            return p;
    }

    return NULL;
}

int sched_available() { return _process_queue->head != 0; }
