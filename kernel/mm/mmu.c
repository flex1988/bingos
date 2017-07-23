#include <stddef.h>
#include <types.h>

#include "hal/isr.h"
#include "kernel.h"
#include "mm/frame.h"
#include "mm/kheap.h"
#include "mm/mem_layout.h"
#include "mm/mmu.h"
#include "multiboot.h"

page_dir_t* _kernel_pd;

uint32_t _pdbr;

extern ptr_t _placement_addr;
extern heap_t *kheap;

static inline void enable_paging() {
    uint32_t r;
    asm volatile("mov %%cr0, %0" : "=r"(r));
    r |= 0x80000001;
    asm volatile("mov %0, %%cr0" ::"r"(r));
}

static inline void setup_pages() { asm volatile("mov %0, %%cr3" ::"r"((uint32_t)_kernel_pd)); }

ptr_t get_physaddr(ptr_t virtualaddr) {
    int pdidx = virtualaddr >> 22;
    int ptidx = virtualaddr >> 12 & 0x03ff;
    int offset = virtualaddr & 0xfff;

    ptr_t page = _kernel_pd->entries[pdidx].addr << 12;

    return page + offset;
}

void page_fault(registers_t regs) {
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    // The error code gives us details of what happened.
    int present = !(regs.err_code & 0x1);  // Page not present
    int rw = regs.err_code & 0x2;          // Write operation?
    int us = regs.err_code & 0x4;          // Processor was in user-mode?
    int reserved = regs.err_code & 0x8;    // Overwritten CPU-reserved bits of page entry?
    int id = regs.err_code & 0x10;         // Caused by an instruction fetch?

    // Output an error message.
    printk("Page fault! ( ) at 0x%x", faulting_address);
    PANIC("Page fault");
}

// get page from page table
page_t* get_page(uint32_t virt, int make, uint32_t flags) {
    uint32_t pdidx = PAGE_DIRECTORY_INDEX(virt);
    ASSERT(pdidx < 1024);
    page_tabl_t* tabl = _kernel_pd->tabls[pdidx];
    if (tabl) {
        return &tabl->pages[PAGE_TABLE_INDEX(virt)];
    } else if (make) {
        // memory before _placement_addr must be identical mapped, so page table always can be accessed
        uint32_t phys;
        page_tabl_t* new = (page_tabl_t*)kmalloc_i(sizeof(page_tabl_t), 1, &phys);
        memset(new, 0, 4096);
        _kernel_pd->tabls[PAGE_DIRECTORY_INDEX(virt)] = new;
        paged_entry_t* entry = (paged_entry_t*)&_kernel_pd->entries[PAGE_DIRECTORY_INDEX(virt)];
        entry->present = 1;
        entry->rw = 1;
        entry->addr = phys >> 12;
        return &new->pages[PAGE_TABLE_INDEX(virt)];
    } else {
        return 0;
    }
}

void page_map(page_t* page, int kernel, int rw) {
    if (page->addr != 0)
        return;
    else {
        memset(page, 0, sizeof(page_t));
        page->rw = rw;
        page->user = kernel ? 0 : 1;
        page->present = 1;
        page->addr = alloc_frame();
    }
}

void page_unmap(page_t* page) {
    free_frame(page->addr);
    memset(page, 0, sizeof(page_t));
}

void page_identical_map(page_t* page, int kernel, int rw, uint32_t virt) {
    if (page->addr != 0)
        return;
    else {
        memset(page, 0, sizeof(page_t));
        page->rw = rw;
        page->user = kernel ? 0 : 1;
        page->present = 1;
        page->addr = virt >> 12;
    }
}

void mmu_init() {
    _kernel_pd = (page_dir_t*)kmalloc_i(sizeof(page_dir_t), 1, 0);
    ASSERT(!((uint32_t)_kernel_pd & 0x00000fff));
    memset(_kernel_pd, 0x0, sizeof(page_dir_t));

    ptr_t phys, virt;
    page_t* page;

    for (virt = KHEAP_START; virt < KHEAP_START + KHEAP_INITIAL_SIZE; virt += 0x1000) {
        get_page(virt, 1, 0);
    }

    virt = 0;
    // idenitify map memory before _placement_addr
    while (virt < _placement_addr + 0x1000) {
        page = get_page(virt, 1, 0);
        ASSERT(page != 0);
        ASSERT(page->addr == 0);
        page_identical_map(page, 0, 0, virt);
        virt += 0x1000;
    }

    // map 0x100000 physic to virtual 0xc0000000
    for (virt = KHEAP_START; virt < KHEAP_START + KHEAP_INITIAL_SIZE; virt += 0x1000) {
        page = get_page(virt, 1, 0);
        ASSERT(page != 0);
        page_map(page, 0, 0);
    }

    register_interrupt_handler(14, page_fault);

    x86_write_cr3((uint32_t)&_kernel_pd->entries);
    x86_write_cr0(x86_read_cr0() | (1 << 31));

    kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);

    printk("paging init...");
}
