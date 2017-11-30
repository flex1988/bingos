#include "lib/tree.h"
#include "kernel.h"
#include "kernel/kheap.h"
#include <string.h>

tree_t *create_tree() {
    tree_t *t = kmalloc(sizeof(tree_t));

    t->length = 0;
    t->root = NULL;

    return t;
}

tree_node_t *create_tree_node(tree_t *t, char *name, int len, void *data) {
    tree_node_t *n = kmalloc(sizeof(tree_node_t));

    n->length = 0;
    n->tree = t;
    n->data = data;

    memcpy(n->name, name, len);

    memset(n->children, 0, sizeof(tree_node_t *) * MAX_TREE_CHILDREN);

    return n;
}

void set_tree_root(tree_t *t, tree_node_t *n) {
    t->root = n;
    t->length++;
}

void tree_node_insert(tree_node_t *p, tree_node_t *n) { p->children[p->length++] = n; }

tree_node_t *tree_search(tree_t *t, char *name) { return tree_node_search(t->root, name); }

tree_node_t *tree_node_search(tree_node_t *n, char *name) {
    if (!strcmp(n->name, name))
        return n;

    tree_node_t *s;
    for (int i = 0; i < n->length; i++) {
        if (s = tree_node_search(n->children[i], name))
            return s;
    }

    return NULL;
}
