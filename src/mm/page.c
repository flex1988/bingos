#include <stddef.h>
#include <types.h>

#include "mm/page.h"
#include "multiboot.h"

static phys_addr_t _placement_addr;
static uint64_t _total_pages;

static uint32_t *_pages;

#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))

static void page_early_alloc(phys_addr_t *phys, size_t size, int align) {
    if (align && (_placement_addr & 0xFFF)) {
        _placement_addr &= 0xFFFFF000;
        _placement_addr += 0x1000;
    }

    if (phys) {
        *phys = _placement_addr;
    }

    _placement_addr += size;
}

void page_init(struct multiboot_info *mbi) {
    phys_addr_t addr;
    phys_size_t mem_size = 0;
    unsigned long long int i;

    if (CHECK_FLAG(mbi->flags, 0)) {
        printk("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);
    }

    if (!CHECK_FLAG(mbi->flags, 6)) {
        printk("mmap not valid!");
        return;
    }

    multiboot_memory_map_t *mmap;

    for (mmap = (multiboot_memory_map_t *)mbi->mmap_addr; mmap < (mbi->mmap_addr + mbi->mmap_length);
         mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(unsigned int))) {
        if (mmap->type == 2) {
            printk("memory  base_addr 0x%x%x length: 0x%x%x",mmap->addr,mmap->len);
            for (i = 0; i < mmap->len; i += 0x1000) {
                if (mmap->addr + i > 0xFFFFFFFF)
                    break;
            }
        }
    }

    _total_pages = mem_size / PAGE_SIZE;

    page_early_alloc(&addr, _total_pages / (4 * 8), 0);

    _pages = (uint32_t *)addr;

    // memset(_pages, 0, _total_pages / (4 * 8));
}
