#include "kernel.h"
#include "kernel/frame.h"
#include "kernel/kheap.h"
#include "kernel/mm.h"
#include "kernel/mmu.h"
#include "kernel/process.h"

#include "libc/errno.h"

extern process_t *_current_process;

void *sys_brk(void *brk) {
    int end_code = _current_process->brk;

    int free;
    uint32_t newbrk, oldbrk;

    if (brk < end_code)
        return _current_process->brk;

    newbrk = PAGE_ALIGN((uint32_t)brk);
    oldbrk = PAGE_ALIGN((uint32_t)_current_process->brk);

    if (oldbrk == newbrk) {
        _current_process->brk = brk;
        return brk;
    }

    if (brk <= _current_process->brk) {
        _current_process->brk = brk;
        do_munmap(newbrk, oldbrk - newbrk);
        return brk;
    }

    free = free_pages();
    if (free < 0) {
        printk("free pages is not enough");
        return _current_process->brk;
    }

    _current_process->brk = brk;

    do_mmap(oldbrk, newbrk - oldbrk);

    return brk;
}

// Insert vm structure into process list
// This makes sure the list is sorted by start address, and
// some simple overlap checking
void insert_vm(process_t *p, vm_area_t *area) {
    vm_area_t **prev, *i;
    prev = &p->mmap;

    for (i = p->mmap; i != NULL; i = i->next) {
        if (i->vm_start > area->vm_start) {
            break;
        }

        prev = &i->next;

        if ((area->vm_start >= i->vm_start && area->vm_start < i->vm_end) || (area->vm_end >= i->vm_start && area->vm_end < i->vm_end)) {
            printk("insert vm: overlaped");
        }
    }

    area->next = i;
    *prev = area;
}

int do_munmap(uint32_t addr, uint32_t len) {
    uint32_t end = addr + len;

    if ((len = PAGE_ALIGN(len)) == 0)
        return 0;

    vm_area_t **npp, *i, *free;

    npp = &_current_process->mmap;

    free = NULL;

    for (i = *npp; i != NULL; i = *npp) {
        if (end <= i->vm_start || addr >= i->vm_end) {
            npp = &i->next;
            continue;
        }

        *npp = i->next;
        i->next = free;
        free = i;
    }

    if (free == NULL)
        return 0;

    while (free) {
        i = free;
        free = free->next;

        kfree(i);
    }

    page_t *page;
    for (uint32_t i = addr; i < addr + len; i += PAGE_SIZE) {
        page = get_page(i, 0, _current_process->pd);
        page_unmap(page);
    }

    return 0;
}

int do_mmap(uint32_t addr, uint32_t len) {
    
    if ((len = PAGE_ALIGN(len)) == 0) {
        printk("page align %d", len);
        return addr;
    }

    // args check

    // clear old maps
    do_munmap(addr, len);

    page_t *page;
    for (uint32_t i = addr; i <= addr + len; i += PAGE_SIZE) {
        page = get_page(i, 1, _current_process->pd);
        page_map(page, 0, 1);
    }

    vm_area_t *area = kmalloc(sizeof(vm_area_t));
    if (!area)
        return -ENOMEM;

    area->vm_start = addr;
    area->vm_end = addr + len;

    insert_vm(_current_process, area);

    // merge_segments();

    return 0;
}

void dump_vm_area(vm_area_t *area) {
    int i = 0;
    while (area) {
        printk("vm %d start: 0x%x end: 0x%x", i++, area->vm_start, area->vm_end);
        area = area->next;
    }
}
