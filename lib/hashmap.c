#include "kernel.h"
#include "kernel/kheap.h"

#include "lib/hashmap.h"

uint32_t hashmap_string_hash(void* key) {
    uint32_t hash = 0;
    int c;
    char* p = (char*)key;

    while ((c = *p++)) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

hashmap_t* hashmap_create(int size, int mode) {
    hashmap_t* map = malloc(sizeof(hashmap_t));
    if (mode == HASHMAP_STRING) {
        map->hash = hashmap_string_hash;
        map->compare = hashmap_string_compare;
        map->key_dup = hashmap_string_dup;
        map->key_free = free;
        map->entry_free = free;
    } else {
        ;  // not implement
    }

    map->size = size;
    map->entries = malloc(sizeof(hashmap_entry_t*) * size);
    memset(map->entries, 0x0, sizeof(hashmap_entry_t*) * size);

    return map;
}

void* hashmap_set(hashmap_t* map, void* key, void* value) {
    uint32_t hash = map->hash(key) % map->size;

    hashmap_entry_t* x = map->entries[hash];
    if (!x) {
        hashmap_entry_t* e = malloc(sizeof(hashmap_entry_t));
        e->key = map->key_dup(key);
        e->value = value;
        e->next = NULL;
        map->entries[hash] = e;
        return NULL;
    } else {
        hashmap_entry_t* p = NULL;
        do {
            if (map->compare(x->key, key)) {
                void* out = x->value;
                x->value = value;
                return out;
            } else {
                p = x;
                x = x->next;
            }
        } while (x);

        hashmap_entry_t* e = malloc(sizeof(hashmap_entry_t));
        e->key = map->key_dup(key);
        e->value = value;
        e->next = NULL;

        p->next = e;
        return NULL;
    }
}

void* hashmap_get(hashmap_t* map, void* key) {
    uint32_t hash = map->hash(key) % map->size;

    hashmap_entry_t* x = map->entries[hash];
    if (!x) {
        return NULL;
    } else {
        do {
            if (map->compare(x->key, key)) {
                return x->value;
            }
            x = x->next;
        } while (x);
        return NULL;
    }
}

void* hashmap_delete(hashmap_t* map, void* key) {
    uint32_t hash = map->hash(key) % map->size;

    hashmap_entry_t* x = map->entries[hash];
    if (!x) {
        return NULL;
    } else {
        if (map->compare(x->key, key)) {
            void* out = x->value;
            map->entries[hash] = x->next;
            map->key_free(x->key);
            map->entry_free(x);
            return out;
        } else {
            hashmap_entry_t* p = x;
            x = x->next;
            do {
                if (map->compare(x->key, key)) {
                    void* out = x->value;
                    p->next = x->next;
                    map->key_free(x->key);
                    map->entry_free(x);
                    return out;
                }

                p = x;
                x = x->next;
            } while (x);
        }

        return NULL;
    }
}

void hashmap_free(hashmap_t* map) {
    for (uint32_t i = 0; i < map->size; i++) {
        hashmap_entry_t* x = map->entries[i];
        hashmap_entry_t* p;

        while (x) {
            p = x;
            x = x->next;
            map->key_free(p->key);
            map->entry_free(p);
        }
    }

    free(map->entries);
}
