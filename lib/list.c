#include "lib/list.h"
#include "kernel.h"
#include "kernel/kheap.h"

list_t *list_create() {
    list_t *new = kmalloc(sizeof(list_t));
    memset(new, 0x0, sizeof(list_t));
    return new;
}

void list_destroy(list_t *l) {
    list_node_t *n = l->head;
    while (n) {
        kfree(n->value);
        n = n->next;
    }
}

void list_remove(list_t *l, int index) {
    if (index > l->length)
        return;

    size_t i = 0;
    list_node_t *n = l->head;
    while (i < index) {
        n = n->next;
        i++;
    }

    list_delete(l, n);
}

void list_delete(list_t *l, list_node_t *n) {
    if (n == l->head) {
        l->head = n->next;
    }

    if (n == l->tail) {
        l->tail = n->prev;
    }

    if (n->prev) {
        n->prev->next = n->next;
    }

    if (n->next) {
        n->next->prev = n->prev;
    }

    l->length--;
}

void list_push_back(list_t *l, void *value) {
    list_node_t *n = kmalloc(sizeof(list_node_t));

    n->value = value;
    n->next = 0;
    n->prev = 0;

    if (!l->tail) {
        l->head = n;
    } else {
        l->tail->next = n;
        n->prev = l->tail;
    }

    l->tail = n;
    l->length++;
}

list_node_t *list_pop_front(list_t *l) {
    if (!l->head)
        return NULL;

    list_node_t *n = l->head;
    list_delete(l, l->head);

    return n;
}
