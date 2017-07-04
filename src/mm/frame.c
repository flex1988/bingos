#include <types.h>

#include "mm/frame.h"

#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))

// pd最后一个entry指向自己，所以pd的地址是0xfffff000
static page_dir_t *_pd = (page_dir_t *)0xfffff000;

void frame_init(struct multiboot_info *mbi) {
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

    for (mmap = (multiboot_memory_map_t *)mbi->mmap_addr; mmap < (multiboot_memory_map_t *)(mbi->mmap_addr + mbi->mmap_length);
         mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(unsigned int))) {
        if (mmap->type == 2) {
            printk("memory  base_addr 0x%x%x length: 0x%x%x", mmap->addr, mmap->len);
            for (i = 0; i < mmap->len; i += 0x1000) {
                if (mmap->addr + i > 0xFFFFFFFF)
                    break;
            }
        }
    }
}

ptr_t get_physaddr(ptr_t virtualaddr) {
    int pdidx = virtualaddr >> 22;
    int ptidx = virtualaddr >> 12 & 0x03ff;
    int offset = virtualaddr & 0xfff;

    page_tabl_t *pt = (page_tabl_t *)(_pd->tabls[pdidx].addr << 12);
    ptr_t page = pt->pages[ptidx].addr << 12;

    return page + offset;
}
