#include "kernel.h"
#include "kernel/malloc.h"
#include "kernel/mmu.h"

typedef struct block_header_s block_header_t;
struct block_header_s {
    uint32_t bh_flags;
    union {
        uint32_t ubh_length;
        block_header_t *fbh_next;
    } vp;
};

#define MAX_KMALLOC_K 4
#define MAX_GET_FREE_PAGE_TRIES 3

#define MF_USED 0xffaa0055
#define MF_FREE 0x0055ffaa

#define bh_length vp.ubh_length
#define bh_next vp.fbh_next
#define BH(p) ((block_header_t *)(p))

typedef struct page_descriptor_s page_descriptor_t;
struct page_descriptor_s {
    page_descriptor_t *next;
    block_header_t *firstfree;
    int order;
    int nfree;
};

typedef struct {
    page_descriptor_t *firstfree;
    int size;
    int nblocks;

    int nmallocs;
    int nfrees;
    int nbytesmalloced;
    int npages;
} size_descriptor_t;

size_descriptor_t sizes[] = {{NULL, 32, 127, 0, 0, 0, 0}, {NULL, 64, 63, 0, 0, 0, 0},  {NULL, 128, 31, 0, 0, 0, 0},
                             {NULL, 252, 16, 0, 0, 0, 0}, {NULL, 508, 8, 0, 0, 0, 0},  {NULL, 1020, 4, 0, 0, 0, 0},
                             {NULL, 2040, 2, 0, 0, 0, 0}, {NULL, 4080, 1, 0, 0, 0, 0}, {NULL, 0, 0, 0, 0, 0, 0}};

#define NBLOCKS(order) (sizes[order].nblocks)
#define BLOCKSIZE(order) (sizes[order].size)

static page_dir_t *_kernel_pd;

int get_order(int size) {
    int order;
    size += sizeof(block_header_t);

    for (order = 0; BLOCKSIZE(order); order++) {
        if (size <= BLOCKSIZE(order))
            return order;
    }
}

static uint32_t kmalloc_start;
static uint32_t kmalloc_end;

void kmalloc_init(uint32_t start, uint32_t size) { kmalloc_start = start; }

static uint32_t __get_free_page() {
    uint32_t ret;
    page_t *page;

    page = get_page(kmalloc_end, 0, _kernel_pd);
    ASSERT(page);
    page_map(page, 1, 0);

    ret = kmalloc_end;
    kmalloc_end += 0x1000;

    return ret;
}

void *kmalloc(size_t size) {
    uint32_t flags;
    int order, i, sz, tries;
    block_header_t *p;
    page_descriptor_t *page;

    if (size > MAX_KMALLOC_K * 1024) {
        printk("kmalloc: I refuse to allocate %d bytes", size);
        return NULL;
    }

    order = get_order(size);
    if (order < 0) {
        printk("kmalloc: allocate too large a block %d bytes", size);
        return NULL;
    }

    tries = MAX_GET_FREE_PAGE_TRIES;
    while (tries--) {
        if ((page = sizes[order].firstfree) && (p = page->firstfree)) {
        }

        sz = BLOCKSIZE(order);

        page = (page_descriptor_t *)__get_free_page();

        if (!page) {
            printk("Coundn't get a free page...");
            return NULL;
        }

        sizes[order].npages++;

        for (i = NBLOCKS(order), p = BH(page + 1); i > 1; i--, p = p->bh_next) {
            // p->bh_flags = MF_FREE;
            p->bh_next = BH(((long)p) + sz);
        }

        p->bh_flags = MF_FREE;
        p->bh_next = NULL;

        page->order = order;
        page->nfree = NBLOCKS(order);
        page->firstfree = BH(page + 1);

        page->next = sizes[order].firstfree;
        sizes[order].firstfree = page;
    }

    return NULL;
}

void kfree(void *p) {}
