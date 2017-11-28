#ifndef __HEAP_H__
#define __HEAP_H__

#include "kernel.h"
#include "kernel/memlayout.h"
#include "kernel/mmu.h"

#include "lib/ordered_array.h"

#define HEAP_INDEX_SIZE 0x2000
#define HEAP_MAGIC 0x12345678

typedef struct {
    uint32_t magic;
    uint8_t hole;
    uint32_t size;
} header_t;

typedef struct {
    uint32_t magic;
    header_t *header;
} footer_t;

typedef struct {
    ordered_array_t index;
    uint32_t start;
    uint32_t end;
    uint32_t max;
    uint8_t readonly;
} heap_t;

heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max);
void *_alloc(uint32_t size, uint8_t align, heap_t *heap);
void _free(void *p, heap_t *heap);

#endif
