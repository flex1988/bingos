#include <heap.h>
#include <stdlib.h>
#include <kernel/memlayout.h>

heap_t *heap = 0;

static void init_user_heap() { heap = create_heap(UHEAP_START, UHEAP_START + UHEAP_INITIAL_SIZE, 0x400000); }

void *malloc(size_t size) {
    if (!heap)
        init_user_heap();

    return _alloc(size, 0, heap);
}

void free(void *p) { _free(p, heap); }
