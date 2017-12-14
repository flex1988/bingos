#ifndef __LIST_H__
#define __LIST_H__

typedef struct list_node list_node_t;
typedef struct list list_t;

struct list_node {
    list_node_t *prev;
    list_node_t *next;
    void *value;
} __attribute__((packed));

struct list {
    list_node_t *head;
    list_node_t *tail;
    int length;
} __attribute__((packed));

void list_destroy(list_t *);
void list_free(list_t *);
void list_insert(list_t *, void *);
list_t *list_create();
list_node_t *list_find(list_t *, void *);
void list_remove(list_t *, int index);
list_node_t *list_pop(list_t *);

void list_push_front(list_t *l, void *value);
void list_push_back(list_t *l, void *value);
list_node_t *list_pop_front(list_t *l);
list_node_t *list_pop_back(list_t *l);

#define foreach(i, list) for (list_node_t *i = list->head; i != NULL; i = i->next)

#endif
