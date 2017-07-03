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

/*void page_init(struct multiboot_info *mbi) {*/
    /*phys_addr_t addr;*/
    /*phys_size_t mem_size = 0;*/

    /*struct multiboot_mmap_entry *mmap;*/
    /*_placement_addr = *((uint32_t *)(mbi->mods_addr + 4));*/

    /*printk("page: placement address at 0x%x\n", _placement_addr);*/

    /*printk("memory ares:\n");*/
    /*for (addr = mbi->mmap_addr; addr < (mbi->mmap_addr + mbi->mmap_length); addr += (mmap->size + sizeof(mmap->size))) {*/
        /*mmap = (struct multiboot_mmap_entry *)addr;*/
        /*mem_size += mmap->len;*/
        /*printk("    start: 0x%x, length: 0x%x\n", mmap->addr, mmap->len);*/
    /*}*/

    /*_total_pages = mem_size / PAGE_SIZE;*/

    /*printk("page: available physical memory size:%uMB, total_pages: %u\n", mem_size / (1024 * 1024), _total_pages);*/

    /*page_early_alloc(&addr, _total_pages / (4 * 8), 0);*/

    /*_pages = (uint32_t *)addr;*/

    /*// memset(_pages, 0, _total_pages / (4 * 8));*/
/*}*/
