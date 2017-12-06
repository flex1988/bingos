#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <types.h>

#ifdef __KERNEL__
#define malloc kmalloc
#define free kfree
#else
#define malloc malloc
#define free free
#endif

typedef uint32_t (*func_hash_t)(void *key);
typedef int (*func_compare_t)(void *a, void *b);
typedef void (*func_free_t)(void *);
typedef void *(*func_dup_t)(void *);

typedef struct hashmap_entry_s hashmap_entry_t;
struct hashmap_entry_s {
    char *key;
    void *value;
    hashmap_entry_t *next;
};

typedef struct hashmap_s hashmap_t;
struct hashmap_s {
    func_hash_t hash;
    func_compare_t compare;
    func_dup_t key_dup;
    func_free_t key_free;
    func_free_t entry_free;
    size_t size;
    hashmap_entry_t **entries;
};

#define HASHMAP_STRING 0
#define HASHMAP_INTEGER 1

hashmap_t *hashmap_create(int size, int mode);

void *hashmap_set(hashmap_t *map, void *key, void *value);
void *hashmap_get(hashmap_t *map, void *key);
void *hashmap_delete(hashmap_t *map, void *key);

int hashmap_exist(hashmap_t *map, void *key);
void hashmap_free(hashmap_t *map);

uint32_t hashmap_string_hash(void *key);
int hashmap_string_compare(void *a, void *b);
void *hashmap_string_dup(void *key);

#endif
