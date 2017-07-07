#include <stddef.h>
#include <types.h>

#include "mm/page.h"
#include "multiboot.h"

// pd最后一个entry指向自己，所以pd的地址是0xfffff000
static page_dir_t *_pd = (page_dir_t *)0xfffff000;

static inline void enable_paging() {}

static inline void setup_pages() {}

void page_init() {
    setup_pages();
    enable_paging();
}

ptr_t get_physaddr(ptr_t virtualaddr) {
    int pdidx = virtualaddr >> 22;
    int ptidx = virtualaddr >> 12 & 0x03ff;
    int offset = virtualaddr & 0xfff;

    page_tabl_t *pt = (page_tabl_t *)(_pd->tabls[pdidx].addr << 12);
    ptr_t page = pt->pages[ptidx].addr << 12;

    return page + offset;
}
