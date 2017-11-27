#include <stddef.h>
#include <types.h>

#include "hal/isr.h"
#include "kernel.h"
#include "kernel/frame.h"
#include "kernel/kheap.h"
#include "kernel/memlayout.h"
#include "kernel/mmu.h"
#include "multiboot.h"

page_dir_t* _kernel_pd = 0;
page_dir_t* _current_pd = 0;

uint32_t _pdbr;

extern ptr_t _placement_addr;
extern heap_t* kheap;
extern void copy_page_physical(uint32_t src, uint32_t dst);

extern uint8_t* frame_buffer;

static inline void page_dir_switch(page_dir_t* dir) {
    _current_pd = dir;
    __asm__ volatile("mov %0, %%cr3" ::"r"(dir->physical));
}

static inline void enable_paging() {
    uint32_t r;
    __asm__ volatile("mov %%cr0, %0" : "=r"(r));
    r |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" ::"r"(r));
}

static inline void setup_pages() { __asm__ volatile("mov %0, %%cr3" ::"r"((uint32_t)_kernel_pd)); }

ptr_t get_physaddr(ptr_t virtualaddr) {
    int pdidx = virtualaddr >> 22;
    int ptidx = virtualaddr >> 12 & 0x03ff;
    int offset = virtualaddr & 0xfff;

    page_tabl_t* tabl = _kernel_pd->tabls[pdidx];
    page_t* page = &tabl->pages[ptidx];

    return page->addr << 12 + offset;
}

void page_fault(registers_t* regs) {
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;
    __asm__ volatile("mov %%cr2, %0" : "=r"(faulting_address));

    // The error code gives us details of what happened.
    int present = !(regs->err_code & 0x1);  // Page not present
    int rw = regs->err_code & 0x2;          // Write operation?
    int us = regs->err_code & 0x4;          // Processor was in user-mode?
    int reserved = regs->err_code & 0x8;    // Overwritten CPU-reserved bits of page entry?
    int id = regs->err_code & 0x10;         // Caused by an instruction fetch?

    // Output an error message.
    printk("Page fault! ( ) at 0x%x", faulting_address);
    PANIC("Page fault");
}

// get page from page table
page_t* get_page(uint32_t virt, int make, page_dir_t* pd) {
    uint32_t pdidx = PAGE_DIRECTORY_INDEX(virt);
    ASSERT(pdidx < 1024);
    page_tabl_t* tabl = pd->tabls[pdidx];

    if (tabl) {
        return &tabl->pages[PAGE_TABLE_INDEX(virt)];
    } else if (make) {
        // memory before _placement_addr must be identical mapped, so page table always can be accessed
        uint32_t phys;
        page_tabl_t* new = (page_tabl_t*)kmalloc_i(sizeof(page_tabl_t), 1, &phys);

        memset(new, 0, 4096);
        pd->tabls[PAGE_DIRECTORY_INDEX(virt)] = new;
        paged_entry_t* entry = (paged_entry_t*)&pd->entries[PAGE_DIRECTORY_INDEX(virt)];
        entry->present = 1;
        entry->rw = 1;
        entry->user = 1;
        entry->addr = phys >> 12;

        return &new->pages[PAGE_TABLE_INDEX(virt)];
    } else {
        return 0;
    }
}

void page_map(page_t* page, int kernel, int rw) {
    ASSERT(page);
    if (page->addr != 0) {
        /*printk("map page already exist frame page->addr 0x%x", page->addr);*/
        /*PANIC("page frame exists");*/
        return;
    } else {
        memset(page, 0, sizeof(page_t));
        page->rw = rw;
        page->user = kernel ? 0 : 1;
        page->present = 1;
        page->addr = alloc_frame();
    }
}

void page_unmap(page_t* page) {
    ASSERT(page);
    free_frame(page->addr);
    page->addr = 0;
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

page_tabl_t* table_clone(page_tabl_t* src, uint32_t* phys) {
    page_tabl_t* tabl = (page_tabl_t*)kmalloc_i(sizeof(page_tabl_t), 1, phys);
    memset(tabl, 0, sizeof(page_tabl_t));

    int i;
    for (i = 0; i < 1024; i++) {
        if (!src->pages[i].addr)
            continue;

        page_map(&tabl->pages[i], 0, 0);

        if (src->pages[i].present)
            tabl->pages[i].present = 1;
        if (src->pages[i].rw)
            tabl->pages[i].rw = 1;
        if (src->pages[i].user)
            tabl->pages[i].user = 1;
        if (src->pages[i].accessed)
            tabl->pages[i].accessed = 1;
        if (src->pages[i].dirty)
            tabl->pages[i].dirty = 1;

        copy_page_physical(src->pages[i].addr * 0x1000, tabl->pages[i].addr * 0x1000);
    }
    return tabl;
}

page_dir_t* page_dir_clone(page_dir_t* src) {
    uint32_t phys;
    page_dir_t* dir = (page_dir_t*)kmalloc_i(sizeof(page_dir_t), 1, &phys);
    memset(dir, 0, sizeof(page_dir_t));

    uint32_t offset = (uint32_t)dir->entries - (uint32_t)dir;
    dir->physical = phys + offset;

    int i;
    for (i = 0; i < 1024; i++) {
        if (!src->tabls[i])
            continue;

        if (_kernel_pd->tabls[i] == src->tabls[i]) {
            dir->tabls[i] = src->tabls[i];
            dir->entries[i] = src->entries[i];
        } else {
            uint32_t tabl_phys;
            /*printk("src 0x%x %d", src->tabls[i], i);*/
            dir->tabls[i] = table_clone(src->tabls[i], &tabl_phys);

            paged_entry_t* entry = &dir->entries[i];
            entry->addr = tabl_phys >> 12;
            entry->present = 1;
            entry->rw = 1;
            entry->user = 1;
        }
    }

    return dir;
}

void mmu_init() {
    _kernel_pd = (page_dir_t*)kmalloc_i(sizeof(page_dir_t), 1, 0);

    ASSERT(!((uint32_t)_kernel_pd & 0x00000fff));
    memset(_kernel_pd, 0x0, sizeof(page_dir_t));

    _kernel_pd->physical = (uint32_t)_kernel_pd->entries;

    ptr_t phys, virt;
    page_t* page;

    for (virt = KHEAP_START; virt < KHEAP_START + KHEAP_INITIAL_SIZE; virt += 0x1000) {
        get_page(virt, 1, _kernel_pd);
    }

    uint32_t fb_length = frame_buffer_length();
    for (virt = frame_buffer; virt < frame_buffer + fb_length; virt += 0x1000) {
        get_page(virt, 1, _kernel_pd);
    }

    virt = 0;
    // idenitify map memory before _placement_addr
    while (virt < _placement_addr + 0x1000) {
        page = get_page(virt, 1, _kernel_pd);
        ASSERT(page != 0);
        ASSERT(page->addr == 0);
        page_map(page, 1, 0);
        virt += 0x1000;
    }

    // map 0x100000 physic to virtual 0xc0000000
    for (virt = KHEAP_START; virt < KHEAP_START + KHEAP_INITIAL_SIZE; virt += 0x1000) {
        page = get_page(virt, 1, _kernel_pd);
        ASSERT(page != 0);
        page_map(page, 1, 0);
    }

    // identical map frame buffer, maybe will cause crash? but whatever
    for (virt = frame_buffer; virt < frame_buffer + fb_length; virt += 0x1000) {
        page = get_page(virt, 1, _kernel_pd);
        ASSERT(page != 0);
        page_identical_map(page, 1, 0, virt);
    }

    register_interrupt_handler(14, page_fault);

    page_dir_switch(_kernel_pd);
    enable_paging();

    kheap = create_heap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);

    _current_pd = page_dir_clone(_kernel_pd);
    page_dir_switch(_current_pd);
    enable_paging();

    printk("paging init...");
}
