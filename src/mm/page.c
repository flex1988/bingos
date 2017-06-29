#include <types.h>

#include "mm/page.h"
#include "multiboot.h"

static phys_addr_t _placement_addr;

void page_init(struct multiboot_info *mbi) {
    phys_addr_t addr;
    phys_size_t mem_size = 0;

    struct multiboot_mmap_entry *mmap;
    _placement_addr = *((uint32_t *)(mbi->mods_addr + 4));

    printk("page: placement address at 0x%x\n", _placement_addr);

    for (addr = mbi->mmap_addr; addr < (mbi->mmap_addr + mbi->mmap_length); addr += (mmap->size + sizeof(mmap->size))) {
        mmap = (struct multiboot_mmap_entry *)addr;
        mem_size += mmap->len;
    }

    printk("page: available physical memory size:%uMB\n", mem_size / (1024 * 1024));
}
