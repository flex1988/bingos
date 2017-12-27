#include <types.h>

typedef struct block_meta_s block_meta_t;
struct block_meta_s {
    size_t size;
    int free;
    int magic;
    block_meta_t *next;
};

#define META_SIZE sizeof(block_meta_t)

#define MAGIC 0x34127856

static void *_base = NULL;

block_meta_t *find_free_block(block_meta_t **last, size_t size) {
    block_meta_t *current = _base;
    while (current && !(current->free && current->size >= size)) {
        *last = current;
        current = current->next;
    }

    return current;
}

block_meta_t *request_space(block_meta_t *last, size_t size) {
    block_meta_t *block;

    block = sbrk(0);
    void *request = sbrk(size + META_SIZE);
    if (request == (void *)-1) {
        return NULL;
    }

    if (last) {
        last->next = block;
    }

    block->size = size;
    block->next = NULL;
    block->free = 0;
    block->magic = MAGIC;

    return block;
}

void *malloc(size_t size) {
    block_meta_t *block;

    if (size <= 0)
        return NULL;

    if (!_base) {
        block = request_space(NULL, size);
        if (!block)
            return NULL;
        _base = block;
    } else {
        block_meta_t *last = _base;
        block = find_free_block(&last, size);
        if (!block) {
            block = request_space(last, size);
            if (!block)
                return NULL;
        } else {
            block->free = 0;
            block->magic = 0x77777777;
        }
    }

    return block + 1;
}

block_meta_t *get_block_ptr(void *ptr) { return (block_meta_t *)ptr - 1; }

void free(void *ptr) {
    if (!ptr)
        return;

    block_meta_t *block_ptr = get_block_ptr(ptr);
    block_ptr->free = 1;
    block_ptr->magic = 0x55555555;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr)
        return malloc(size);

    block_meta_t *block_ptr = get_block_ptr(ptr);
    if (block_ptr->size >= size) {
        return ptr;
    }

    void *new_ptr;

    new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;

    memcpy(new_ptr, ptr, block_ptr->size);
    free(ptr);

    return new_ptr;
}

void *calloc(size_t nelem, size_t elsize) {
    size_t size = nelem * elsize;
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}
