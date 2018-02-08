#include "kernel.h"
#include "kernel/process.h"
#include "lib/list.h"

volatile list_t *_process_queue;
volatile list_t *_finished_queue;
volatile list_t *_sleep_queue;

extern int malloc_debug;

void sched_init() {
    _process_queue = list_create();
    _finished_queue = list_create();
    _sleep_queue = list_create();
    printk("sched_init...");
}

void sched_enqueue(process_t *p) {
    ASSERT(p);
    list_push_front(_process_queue, (void *)p);
}

void sleep_enqueue(process_t *process, uint32_t seconds, uint32_t subseconds) {
    list_node_t *prev = NULL;

    foreach(node, _sleep_queue) {
        process_t *p = (process_t *)node->value;
        if (p->end_tick > seconds ||
            (p->end_tick == seconds && p->end_subtick > subseconds))
            break;
        prev = node;
    }

    process->end_tick = seconds;
    process->end_subtick = subseconds;

    list_insert_after(_sleep_queue, prev, process);
}

process_t *sched_dequeue() {
    list_node_t *n = list_pop_back(_process_queue);

    if (!n)
        return NULL;

    process_t *p = (process_t *)n->value;

    kfree(n);

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
        if (p->id == pid) {
            list_delete(_finished_queue, n);
            kfree(n);
            return p;
        }
    }

    return NULL;
}

int sched_available() { return _process_queue->head != 0; }
