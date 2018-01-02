#include "kernel.h"
#include "kernel/frame.h"
#include "kernel/malloc.h"
#include "kernel/memlayout.h"
#include "kernel/mmu.h"

#define SKIP_MAX_LEVEL 6
#define SKIP_P INT32_MAX

typedef struct block_header_s {
    uint32_t bh_flags;
    uint32_t magic;
    union {
        uint32_t ubh_length;
        struct block_header_s *fbh_next;
    } vp;
} block_header_t;

#define bh_length vp.ubh_length
#define bh_next vp.fbh_next
#define BH(p) ((block_header_t *)(p))

// huge block is block which size bigger than 4080 byte
typedef struct huge_block_header_s {
    struct huge_block_header_s *next;
    void *head;
    uint32_t size;
    uint32_t magic;
    struct huge_block_header_s *prev;
    struct huge_block_header_s *forward[SKIP_MAX_LEVEL + 1];
} huge_block_header_t;

typedef struct huge_block_skip_list_s {
    huge_block_header_t head;
    int level;
} huge_block_skip_list_t;

#define MAX_KMALLOC_K 4
#define MAX_GET_FREE_PAGE_TRIES 3

#define MF_USED 0xffaa0055
#define MF_FREE 0x0055ffaa

#define BLOCK_MAGIC 0x33445566

typedef struct page_descriptor_s {
    struct page_descriptor_s *next;
    block_header_t *firstfree;
    int order;
    int nfree;
} page_descriptor_t;

#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PAGE_DESC(p) ((page_descriptor_t *)((uint32_t)(p)&PAGE_MASK))

typedef struct size_descriptor_s {
    struct size_descriptor_s *firstfree;
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

static huge_block_skip_list_t __huge_blocks;

static uint32_t __kmalloc(size_t size);

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

void kmalloc_init(uint32_t start, uint32_t size) {
    kmalloc_start = kmalloc_end = start;
    printk("kmalloc init...");
}

static uint32_t __get_free_page() {
    uint32_t ret;

    if (kmalloc_end & 0xfffff000) {
        kmalloc_end &= 0xfffff000;
        kmalloc_end += 0x1000;
    }

    ret = kmalloc_end;
    kmalloc_end += 0x1000;

    return ret;
}

static uint32_t __get_free_pages(uint32_t pages) {
    uint32_t ret;

    if (kmalloc_end & 0xfffff000) {
        kmalloc_end &= 0xfffff000;
        kmalloc_end += 0x1000;
    }

    ret = kmalloc_end;
    kmalloc_end += pages * 0x1000;

    return ret;
}

uint32_t kmalloc_i(size_t size, int align, uint32_t *phys) {
    if (kmalloc_start) {
        void *addr = __kmalloc(size);

        if (phys) {
            *phys = (uint32_t)get_physaddr((uint32_t)addr);
        }

        return (uint32_t)addr;
    } else {
        return pre_alloc(size, align, phys);
    }
}

uint32_t kmalloc(size_t size) { return kmalloc_i(size, 0, 0); }
void kfree(void *p) { return __kfree(p); }

static uint32_t skip_rand(void) {
    static uint32_t x = 123456789;
    static uint32_t y = 362436069;
    static uint32_t z = 521288629;
    static uint32_t w = 88675123;

    uint32_t t;

    t = x ^ (x << 11);
    x = y;
    y = z;
    z = w;
    return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

static int __kmalloc_random_level(void) {
    int level = 0;
    while (skip_rand() < SKIP_P && level < SKIP_MAX_LEVEL) {
        level++;
    }

    return level;
}

static huge_block_header_t *__kmalloc_skip_list_find_huge_block(uint32_t size) {
    huge_block_header_t *node = &__huge_blocks.head;

    int i;
    for (i = __huge_blocks.level; i >= 0; --i) {
        while (node->forward[i] && (node->forward[i]->size < size)) {
            node = node->forward[i];
        }
    }

    node = node->forward[0];
    return node;
}

static void __kmalloc_skip_list_insert(huge_block_header_t *n) {
    huge_block_header_t *node = &__huge_blocks.head;
    huge_block_header_t *update[SKIP_MAX_LEVEL + 1];

    int i;
    for (i = __huge_blocks.level; i >= 0; --i) {
        while (node->forward[i] && node->forward[i]->size < n->size) {
            node = node->forward[i];
        }
        update[i] = node;
    }
    node = node->forward[0];

    if (node != n) {
        int level = __kmalloc_random_level();
        if (level > __huge_blocks.level) {
            for (i = __huge_blocks.level + 1; i <= level; ++i) {
                update[i] = &__huge_blocks.head;
            }
            __huge_blocks.level = level;
        }

        node = n;
        for (i = 0; i <= level; ++i) {
            node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = node;
        }
    }
}

static uint32_t __kmalloc(size_t size) {
    uint32_t flags;
    int order, i, sz, tries;
    block_header_t *p;
    page_descriptor_t *page;

    if (size > 4080) {
        huge_block_header_t *header = __kmalloc_skip_list_find_huge_block(size);
        if (header) {
            return (uint32_t)header + 0x1000;
        } else {
            uint32_t pages = (size + sizeof(huge_block_header_t)) / PAGE_SIZE + 2;
            header = __get_free_pages(pages);
            header->magic = BLOCK_MAGIC;
            header->size = pages * (PAGE_SIZE - 1);

            return (uint32_t)header + 0x1000;  //(align ? 0x1000 : sizeof(huge_block_header_t));
        }
    }

    order = get_order(size);
    if (order < 0) {
        printk("kmalloc: allocate too large a block %d bytes", size);
        return NULL;
    }

    tries = MAX_GET_FREE_PAGE_TRIES;
    while (tries--) {
        if ((page = sizes[order].firstfree) && (p = page->firstfree)) {
            if (p->bh_flags == MF_FREE) {
                page->firstfree = p->bh_next;
                page->nfree--;

                if (!page->nfree) {
                    sizes[order].firstfree = page->next;
                    page->next = NULL;
                }

                sizes[order].nmallocs++;
                sizes[order].nbytesmalloced += size;

                p->bh_flags = MF_USED;
                p->bh_length = size;
                return p + 1;
            }

            printk("blocks on freelist at 0x%x isn't free size 0x%x", p, size);
            return NULL;
        }

        sz = BLOCKSIZE(order);

        page = (page_descriptor_t *)__get_free_page();

        if (!page) {
            printk("Coundn't get a free page...");
            return NULL;
        }

        sizes[order].npages++;

        for (i = NBLOCKS(order), p = BH(page + 1); i > 1; i--, p = p->bh_next) {
            p->bh_flags = MF_FREE;
            p->magic = BLOCK_MAGIC;
            p->bh_next = BH(((uint32_t)p) + sz);
        }

        p->bh_flags = MF_FREE;
        p->magic = BLOCK_MAGIC;
        p->bh_next = NULL;

        page->order = order;
        page->nfree = NBLOCKS(order);
        page->firstfree = BH(page + 1);

        page->next = sizes[order].firstfree;
        sizes[order].firstfree = page;
    }

    return NULL;
}

void __kfree(void *p) {
    uint32_t size;
    block_header_t *header = (block_header_t *)p - 1;
    if (header->magic == BLOCK_MAGIC) {
        int order;
        page_descriptor_t *page;

        page = PAGE_DESC(header);
        order = page->order;
        if (order < 0) {
            printk("kfree of non-kmalloced memory.");
            return;
        }

        size = header->bh_length;
        header->bh_flags = MF_FREE;
        header->bh_next = page->firstfree;

        page->firstfree = header;
        page->nfree++;

        if (page->nfree == 1) {
            if (page->next) {
                printk("page already on freelist.");
            } else {
                page->next = sizes[order].firstfree;
                sizes[order].firstfree = page;
            }
        }

        if (page->nfree == NBLOCKS(page->order)) {
            ;  // if page is completely free ,free it
        }

        sizes[order].nfrees++;
        sizes[order].nbytesmalloced -= size;
    } else {
        huge_block_header_t *hheader = (huge_block_header_t *)((uint32_t)p - 0x1000);
        ASSERT(hheader->magic == BLOCK_MAGIC);

        __kmalloc_skip_list_insert(hheader);
    }
}
