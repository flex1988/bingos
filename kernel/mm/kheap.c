#include "mm/kheap.h"
#include "kernel.h"
#include "mm/mmu.h"

heap_t *kheap = 0;

uint32_t kmalloc_i(uint32_t size, int align, uint32_t *phys) {
    if (kheap != 0) {
        void *addr = alloc();
        if (phys) {
            *phys = (uint32_t)get_physaddr((ptr_t)addr);
        }
        return (uint32_t)addr;
    } else {
        return pre_alloc(size, align);
    }
}

uint32_t kmalloc(uint32_t size) { return kmalloc_i(size, 0, 0); }

static int32_t find_smallest_hole(uint32_t size, uint8_t align, heap_t *heap) {
    uint32_t iterator = 0;
    while (iterator < heap->index.size) {
        header_t *header = (header_t *)lookup_ordered_array(iterator, &heap->index);

        if (aligin > 0) {
            uint32_t location = (uint32_t)header;
            int32_t offset = 0;
            if ((location + sizeof(header_t)) & 0xfffff000 != 0)
                offset = 0x1000 - (location + sizeof(header_t)) % 0x1000;
            int32_t hole_size = (int32_t)header->size - offset;
            if (hole_size >= size)
                break;
        } else if (header->size >= size) {
            break;
        }
        iterator++;
    }

    if (iterator == heap->index.size)
        return -1;
    else
        return iterator;
}

static int8_t header_comparer(void *a, void *b) { return ((header_t *)a)->size < ((header_t *)b)->size ? 1 : 0; }

heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max, uint8_t supervisor, uint8_t readonly) {
    heap_t *heap = (heap_t *)kmalloc(sizeof(heap_t));

    // heap->index =
    return heap;
}
