#ifndef __TREE_H__
#define __TREE_H__

#define MAX_TREE_CHILDREN 10

typedef struct tree_node_s tree_node_t;
typedef struct tree_s tree_t;

struct tree_node_s {
    int length;
    char name[64];
    void *data;
    tree_t *tree;
    tree_node_t *children[MAX_TREE_CHILDREN];
};

struct tree_s {
    int length;
    tree_node_t *root;
};

tree_t *create_tree();
tree_node_t *create_tree_node(tree_t *t, void *data);
tree_node_t *tree_node_insert(tree_node_t *p, char *name, int len, void *data);
tree_node_t *tree_search(tree_t *t, char *name);
tree_node_t *tree_node_search(tree_node_t *n, void *data);
void set_tree_root(tree_t *t, tree_node_t *n);
#endif
