#include <heap.h>
#include <syscall.h>
#include <types.h>

static uint32_t _brk = 0;

static int32_t find_smallest_hole(uint32_t size, uint8_t align, heap_t* heap) {
    uint32_t iterator = 0;
    while (iterator < heap->index.size) {
        header_t* header = (header_t*)lookup_ordered_array(iterator, &heap->index);

        if (align > 0) {
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

static void expand(uint32_t new, heap_t* heap) {
    void* p = brk(0);

    int ret = brk(p + new);
    if (ret > p) {
        heap->end = heap->start + new;
    }
}

static int8_t header_comparer(void* a, void* b) { return ((header_t*)a)->size < ((header_t*)b)->size ? 1 : 0; }

heap_t* create_heap(uint32_t start, uint32_t end, uint32_t max) {
    _brk = brk(0);
    _brk = brk((void*)_brk + end - start);

    heap_t* heap = (heap_t *)_brk;
    _brk = (heap_t*)brk(_brk + sizeof(heap_t));

    heap->index = place_ordered_array((void*)start, HEAP_INDEX_SIZE, header_comparer);
    
    start += sizeof(type_t) * HEAP_INDEX_SIZE;

    if (start & 0xfffff000 != 0) {
        start &= 0xfffff000;
        start += 0x1000;
    }

    heap->start = start;
    heap->end = end;
    heap->max = max;

    header_t* hole = (header_t*)start;
    hole->size = end - start;
    hole->magic = HEAP_MAGIC;
    hole->hole = 1;

    insert_ordered_array((void*)hole, &heap->index);
    
    return heap;
}

static uint32_t contract(uint32_t new, heap_t* heap) {
    if (new & 0x1000) {
        new &= 0x1000;
        new += 0x1000;
    }

    if (new < HEAP_MIN_SIZE)
        new = HEAP_MIN_SIZE;

    uint32_t old = heap->end - heap->start;

    _brk = brk(_brk + new);

    heap->end = heap->start + new;
    return new;
}

void* _alloc(uint32_t size, uint8_t align, heap_t* heap) {
    uint32_t total = size + sizeof(header_t) + sizeof(footer_t);

    int32_t iterator = find_smallest_hole(total, align, heap);

    if (iterator == -1) {
        uint32_t old_length = heap->end - heap->start;
        uint32_t old_end = heap->end;

        expand(old_length + total, heap);

        uint32_t new_length = heap->end - heap->start;

        iterator = 0;

        uint32_t idx = -1;
        uint32_t value = 0x0;
        while (iterator < heap->index.size) {
            uint32_t tmp = (uint32_t)lookup_ordered_array(iterator, &heap->index);
            if (tmp > value) {
                value = tmp;
                idx = iterator;
            }
            iterator++;
        }

        if (idx == -1) {
            header_t* header = (header_t*)old_end;
            header->magic = HEAP_MAGIC;
            header->size = new_length - old_length;
            header->hole = 1;
            footer_t* footer = (footer_t*)(old_end + header->size - sizeof(footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_ordered_array((void*)header, &heap->index);
        } else {
            header_t* header = lookup_ordered_array(idx, &heap->index);
            header->size += new_length - old_length;

            footer_t* footer = (footer_t*)((uint32_t)header + header->size - sizeof(footer_t));
            footer->header = header;
            footer->magic = HEAP_MAGIC;
        }

        return _alloc(size, align, heap);
    }

    header_t* hole = (header_t*)lookup_ordered_array(iterator, &heap->index);
    uint32_t pos = (uint32_t)hole;
    uint32_t hole_size = hole->size;

    if (hole_size - total < sizeof(header_t) + sizeof(footer_t)) {
        size += hole_size - total;
        total = hole_size;
    }

    if (align && pos & 0xfffff000) {
        uint32_t location = pos + 0x1000 - (pos & 0xfff) - sizeof(header_t);

        header_t* hole_header = (header_t*)pos;
        hole_header->size = 0x1000 - pos & 0xfff - sizeof(header_t);
        hole_header->magic = HEAP_MAGIC;
        hole_header->hole = 1;

        footer_t* hole_footer = (footer_t*)((uint32_t)location - sizeof(footer_t));
        hole_footer->magic = HEAP_MAGIC;
        hole_footer->header = hole_header;

        pos = location;
        hole_size = hole_size - hole_header->size;
    } else {
        remove_ordered_array(iterator, &heap->index);
    }

    header_t* block_header = (header_t*)pos;
    block_header->magic = HEAP_MAGIC;
    block_header->hole = 0;
    block_header->size = total;

    footer_t* block_footer = (footer_t*)(pos + sizeof(header_t) + size);
    block_footer->magic = HEAP_MAGIC;
    block_footer->header = block_header;

    if (hole_size - total > 0) {
        // need to split hole
        header_t* hole_header = (header_t*)(pos + sizeof(header_t) + size + sizeof(footer_t));
        hole_header->magic = HEAP_MAGIC;
        hole_header->hole = 1;
        hole_header->size = hole_size - total;
        footer_t* hole_footer = (footer_t*)((uint32_t)hole_header + hole_size - total - sizeof(footer_t));

        if ((uint32_t)hole_footer < heap->end) {
            hole_footer->magic = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }

        insert_ordered_array((void*)hole_header, &heap->index);
    }

    return (void*)((uint32_t)block_header + sizeof(header_t));
}

void _free(void* p, heap_t* heap) {
    if (p == 0)
        return;

    header_t* header = (header_t*)((uint32_t)p - sizeof(header_t));
    footer_t* footer = (footer_t*)((uint32_t)header + header->size - sizeof(footer_t));

    header->hole = 1;

    char do_add = 1;

    // unify left
    footer_t* test_footer = (footer_t*)((uint32_t)header - sizeof(footer_t));
    if (test_footer->magic == HEAP_MAGIC && test_footer->header->hole == 1) {
        uint32_t cache_size = header->size;
        header = test_footer->header;
        footer->header = header;
        header->size += cache_size;
        do_add = 0;
    }

    // unify right
    header_t* test_header = (header_t*)((uint32_t)footer + sizeof(footer_t));
    if (test_header->magic == HEAP_MAGIC && test_header->hole) {
        header->size += test_header->size;
        test_footer = (footer_t*)((uint32_t)test_header + test_header->size - sizeof(footer_t));
        footer = test_footer;
        uint32_t iterator = 0;
        while ((iterator < heap->index.size) && (lookup_ordered_array(iterator, &heap->index) != (void*)test_header)) {
            iterator++;
        }

        remove_ordered_array(iterator, &heap->index);
    }

    if ((uint32_t)footer + sizeof(footer_t) == heap->end) {
        uint32_t old_length = heap->end - heap->start;
        uint32_t new_length = contract((uint32_t)header - heap->start, heap);

        if (header->size - (old_length - new_length) > 0) {
            header->size -= old_length - new_length;
            footer = (footer_t*)((uint32_t)header + header->size - sizeof(footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
        } else {
            uint32_t iterator = 0;
            while ((iterator < heap->index.size) && (lookup_ordered_array(iterator, &heap->index) != (void*)test_header)) {
                iterator++;
            }

            if (iterator < heap->index.size)
                remove_ordered_array(iterator, &heap->index);
        }
    }

    if (do_add == 1)
        insert_ordered_array((void*)header, &heap->index);
}
