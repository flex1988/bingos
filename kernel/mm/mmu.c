#include <stddef.h>
#include <types.h>

#include "kernel.h"
#include "mm/frame.h"
#include "mm/mem_layout.h"
#include "mm/mmu.h"
#include "hal/isr.h"
#include "multiboot.h"

// pd最后一个entry指向自己，所以pd的地址是0xfffff000
page_dir_t* _pd;
uint32_t _pdbr;

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

void mmu_init() {
    _pd = (page_dir_t*)alloc_frame();
    memset(_pd, 0x0, sizeof(page_dir_t));

    ptr_t phys, virt, i;

    page_tabl_t* tabl = alloc_frame();
    memset(tabl, 0, sizeof(page_tabl_t));

    page_tabl_t* tabl2 = alloc_frame();
    memset(tabl, 0, sizeof(page_tabl_t));

    // idenitify map first 4MB
    for (i = 0, phys = 0x0, virt = 0x0; i < 1024; i++, phys += PAGE_SIZE, virt += PAGE_SIZE) {
        uint32_t* page = &tabl->pages[PAGE_TABLE_INDEX(virt)];
        *page = 0;
        *page |= 0x11;
        *page |= phys;
    }

    // map 0x100000 physic to virtual 0xc0000000
    for (i = 0, phys = 0x100000, virt = 0xc0000000; i < 1024; i++, phys += PAGE_SIZE, virt += PAGE_SIZE) {
        uint32_t* page = &tabl2->pages[PAGE_TABLE_INDEX(virt)];
        *page = 0;
        *page |= 0x11;
        *page |= phys;
    }

    paged_entry_t* paged = &_pd->tabls[PAGE_DIRECTORY_INDEX(0x00000000)];
    paged->present = 1;
    paged->rw = 1;
    paged->addr = (ptr_t)tabl >> 12;

    page_t* page = &_pd->tabls[PAGE_DIRECTORY_INDEX(0xc0000000)];
    page->present = 1;
    page->rw = 1;
    page->addr = (ptr_t)tabl2 >> 12;

    register_interrupt_handler(14, page_fault);

    x86_write_cr3(_pd);
    x86_write_cr0(x86_read_cr0() | (1 << 31));

    printk("paging init...");
}
