#include <types.h>

#include "mm/frame.h"

#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))
#define FRAME_SIZE 4096

static uint64_t _total_memory_size = 0;
static uint32_t _used_frames = 0;
static uint32_t _max_frames = 0;
static uint32_t *_frame_map = 0;
static ptr_t _placement_addr;
extern uint32_t kernel_end;

// pd最后一个entry指向自己，所以pd的地址是0xfffff000
static page_dir_t *_pd = (page_dir_t *)0xfffff000;

inline void bitmap_set(int bit) { _frame_map[bit / 32] |= (1 << (bit % 32)); }

inline void bitmap_unset(int bit) { _frame_map[bit / 32] &= ~(1 << (bit % 32)); }

inline int bitmap_test(int bit) { return _frame_map[bit / 32] & (1 << (bit % 32)); }

static inline uint32_t get_frame_count() { return _max_frames; }

static ptr_t pre_alloc(size_t size) {
    ptr_t t = _placement_addr;
    _placement_addr += size;
    return t;
}

void memory_region_init(ptr_t base, uint64_t size) {
    int bit = base >> 12;
    int frames = size >> 12;

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

int get_first_frame() {
    uint32_t i, j;

    for (i = 0; i < get_frame_count() / 32; i++) {
        if (_frame_map[i] != 0xffffffff) {
            for (j = 0; j < 32; j++) {
                if (!(_frame_map[i] & (1 << j)))
                    return i * 32 + j;
            }
        }
    }
}

ptr_t alloc_frame() {
    if (get_frame_count() <= 0)
        return 0;

    int frame = get_first_frame();

    if (frame == -1)
        return 0;

    bitmap_set(frame);

    _used_frames++;

    return frame << 12;
}

void frame_init(struct multiboot_info *mbi) {
    phys_addr_t addr;
    uint64_t mem_size = 0;
    unsigned long long int i;

    if (!CHECK_FLAG(mbi->flags, 0)) {
        return;
    }

    printk("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    if (!CHECK_FLAG(mbi->flags, 6)) {
        printk("mmap not valid!");
        return;
    }

    if (mbi->mods_count > 0) {
        multiboot_module_t *mod = (multiboot_module_t *)mbi->mods_addr + mbi->mods_count - 1;
        _placement_addr = mod->mod_end;
    } else {
        _placement_addr = 0x200000;
    }

    mem_size = mbi->mem_lower << 10 + mbi->mem_upper << 10;

    _total_memory_size = mem_size;
    _max_frames = _total_memory_size >> 12;
    _used_frames = _max_frames;

    // pre_alloc frame bitmap
    _frame_map = pre_alloc((_max_frames / 8) * sizeof(char));

    memset(_frame_map, 0xf, _used_frames);

    multiboot_memory_map_t *mmap;
    for (mmap = (multiboot_memory_map_t *)mbi->mmap_addr; mmap < (multiboot_memory_map_t *)(mbi->mmap_addr + mbi->mmap_length);
         mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(unsigned int))) {
        if (mmap->type == 1) {
            printk("memory_region_init  base_addr 0x%x%x length: 0x%x%x type: %d", mmap->addr, mmap->len, mmap->type);
            memory_region_init(mmap->addr, mmap->len);
        }
    }

    bitmap_set(0);  // protect kernel memory

    printk("total memory size: 0x%x%x max frames 0x%x used frames 0x%x", _total_memory_size, _max_frames, _used_frames);

    printk("alloc frame 0x%x", alloc_frame());
    printk("alloc frame 0x%x", alloc_frame());
    printk("alloc frame 0x%x", alloc_frame());
}

ptr_t get_physaddr(ptr_t virtualaddr) {
    int pdidx = virtualaddr >> 22;
    int ptidx = virtualaddr >> 12 & 0x03ff;
    int offset = virtualaddr & 0xfff;

    page_tabl_t *pt = (page_tabl_t *)(_pd->tabls[pdidx].addr << 12);
    ptr_t page = pt->pages[ptidx].addr << 12;

    return page + offset;
}
