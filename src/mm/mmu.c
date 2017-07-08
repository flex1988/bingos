#include <stddef.h>
#include <types.h>

#include "kernel.h"
#include "mm/frame.h"
#include "mm/mem_layout.h"
#include "mm/mmu.h"
#include "multiboot.h"

// pd最后一个entry指向自己，所以pd的地址是0xfffff000
static page_dir_t* _pd;

extern ptr_t _placement_addr;

static inline void enable_paging() {
    uint32_t r;
    asm volatile("mov %%cr0, %0" : "=r"(r));
    r |= 0x80000001;
    asm volatile("mov %0, %%cr0" ::"r"(r));
}

static inline void setup_pages() { asm volatile("mov %0, %%cr3" ::"r"((uint32_t)_pd)); }

ptr_t get_physaddr(ptr_t virtualaddr) {
    int pdidx = virtualaddr >> 22;
    int ptidx = virtualaddr >> 12 & 0x03ff;
    int offset = virtualaddr & 0xfff;

    page_tabl_t* pt = (page_tabl_t*)(_pd->tabls[pdidx].addr << 12);
    ptr_t page = pt->pages[ptidx].addr << 12;

    return page + offset;
}

void page_alloc(page_t* page, int flags) {
    if (page->addr != 0) {
        PANIC("alloc page in use");
    } else {
        page->addr = alloc_frame();
        page->present = 1;
    }
}

page_t* mmu_get_page(ptr_t virtual, int alloc, int flags) {
    page_t* page;

    uint32_t pdidx, ptidx;

    pdidx = virtual >> 22;
    ptidx = (virtual >> 12) & 0x03ff;

    if (_pd->tabls[pdidx].present) {
        page_tabl_t* pt = (page_tabl_t*)(_pd->tabls[pdidx].addr << 12);
        page = &pt->pages[ptidx];
    } else if (alloc) {
        page_tabl_refer_t* ref = &_pd->tabls[pdidx];
        page_tabl_t* pt = pre_alloc(sizeof(page_tabl_t), 1);
        memset(pt, 0, sizeof(page_tabl_t));
        ref->addr = (uint32_t)pt >> 12;
        ref->present = 1;
        ref->rw = 1;

        page = &pt->pages[ptidx];
    } else {
        page = NULL;
    }

    return page;
}

void mmu_init() {
    _pd = (page_dir_t*)pre_alloc(sizeof(page_dir_t), 1);
    memset(_pd, 0x0, sizeof(page_dir_t));

    ptr_t i;

    page_t* page;
    for (i = 0; i <  0x110000 + PAGE_SIZE; i += PAGE_SIZE) {
        page = mmu_get_page(i, 1, 0);
        page_alloc(page, 0);
        page->user = 0;
        page->rw = 0;
    }

    for (i = KERNEL_KMEM_START; i < KERNEL_KMEM_START + KERNEL_KMEM_SIZE; i += PAGE_SIZE) {
        page = mmu_get_page(i, 0, 0);
        page_alloc(page, 0);
        page->user = 0;
        page->rw = 0;
    }

    x86_write_cr3(_pd);

    x86_write_cr0(x86_read_cr0() | (1 << 31));

    printk("paging init...");
}
