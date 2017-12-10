#include <types.h>

#include "kernel.h"
#include "kernel/frame.h"
#include "kernel/mmu.h"

#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))
#define FRAME_SIZE 4096

static uint64_t _total_memory_size = 0;
static uint32_t _used_frames = 0;
static uint32_t _max_frames = 0;
static uint32_t *_frame_map = 0;

extern void *kernel_end;
ptr_t _placement_addr = (ptr_t)&kernel_end;

void bitmap_set(int bit) { _frame_map[bit / 32] |= (1 << (bit % 32)); }

void bitmap_unset(int bit) { _frame_map[bit / 32] &= ~(1 << (bit % 32)); }

int bitmap_test(int bit) { return _frame_map[bit / 32] & (1 << (bit % 32)); }

static inline uint32_t get_frame_count() { return _max_frames; }

ptr_t pre_alloc(size_t size, int align, uint32_t *phys) {
    ptr_t t;

    if (align && (_placement_addr & 0xfff)) {
        _placement_addr &= 0xfffff000;
        _placement_addr += 0x1000;
    }

    if (phys) {
        *phys = _placement_addr;
    }

    t = _placement_addr;
    _placement_addr += size;
    return t;
}

void memory_region_init(ptr_t base, uint64_t size) {
    uint32_t bit = base >> 12;
    uint64_t frames = size >> 12;

    for (; frames > 0; frames--) {
        bitmap_unset(bit++);
        _used_frames--;
    }
}

void memory_region_deinit(ptr_t base, uint64_t size) {
    int align = base / FRAME_SIZE;
    int frames = size / FRAME_SIZE;

    for (; frames > 0; frames--) {
        bitmap_set(align++);
        _used_frames++;
    }
}

uint32_t get_first_frame() {
    uint32_t i, j;

    for (i = 0; i < get_frame_count(); i++) {
        if (_frame_map[i] != 0xffffffff) {
            for (j = 0; j < 32; j++) {
                if (!(_frame_map[i] & (1 << j)))
                    return i * 32 + j;
            }
        }
    }
}

uint32_t get_first_frames(size_t size) {
    uint32_t i, j;

    for (i = 0; i < get_frame_count(); i++) {
        if (_frame_map[i] != 0xffffffff) {
            for (j = 0; j < 32; j++) {
                if (!(_frame_map[i] & (1 << j))) {
                    int bit = i * 32;
                    bit += (1 << j);

                    uint32_t free = 0;
                    uint32_t count = 0;
                    for (count = 0; count <= size; count++) {
                        if (bitmap_test(bit + count))
                            free++;
                        if (free == size)
                            return i * 32 + j;
                    }
                }
            }
        }
    }

    return -1;
}

uint32_t alloc_frame() {
    if (get_frame_count() <= 0) {
        PANIC("no more frames");
    }

    uint32_t frame = get_first_frame();

    if (frame == -1) {
        PANIC("get free frame failed");
    }

    bitmap_set(frame);

    _used_frames++;

    return frame;
}

void free_frame(uint32_t frame) {
    bitmap_unset(frame);

    _used_frames--;
}

uint32_t free_pages() { return _max_frames - _used_frames; }

uint32_t alloc_frames(size_t size) {
    if (get_frame_count() <= 0) {
        PANIC("no more frames");
    }

    uint32_t frame = get_first_frames(size);

    if (frame == -1) {
        PANIC("get free frame failed");
    }

    uint32_t i;
    for (i = 0; i < size; i++) {
        bitmap_set(frame + i);
    }
    _used_frames += size;

    return frame;
}

void frame_init(struct multiboot_info *mbi) {
    unsigned long long int i;

    printk("_placement_addr 0x%x", _placement_addr);

    if (!CHECK_FLAG(mbi->flags, 0)) {
        return;
    }

    printk("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    if (!CHECK_FLAG(mbi->flags, 6)) {
        printk("mmap not valid!");
        return;
    }

    multiboot_memory_map_t *mmap;
    for (mmap = (multiboot_memory_map_t *)mbi->mmap_addr; mmap < (multiboot_memory_map_t *)(mbi->mmap_addr + mbi->mmap_length);
         mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(mmap->size))) {
        _total_memory_size += mmap->len;
    }

    _max_frames = (uint32_t)(_total_memory_size >> 12);

    // pre_alloc frame bitmap
    _frame_map = pre_alloc(_max_frames / 32, 0, 0);
    memset(_frame_map, 0x0, _max_frames / 8);

    printk("total memory size: 0x%x%x max frames 0x%x used frames 0x%x", _total_memory_size, _max_frames, _used_frames);

    printk("frame init...");
}
