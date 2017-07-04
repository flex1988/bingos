#include <stddef.h>
#include <types.h>

#include "mm/page.h"
#include "multiboot.h"

static phys_addr_t _placement_addr;
static uint64_t _total_pages;

static uint32_t *_pages;

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

    multiboot_memory_map_t *mmap;

    _total_pages = mem_size / PAGE_SIZE;

    page_early_alloc(&addr, _total_pages / (4 * 8), 0);

    _pages = (uint32_t *)addr;
}
