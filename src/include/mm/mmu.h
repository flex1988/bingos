#ifndef __MMU_H__
#define __MMU_H__

#define PAGE_SIZE (4096)

/* Read CR0 */
static inline uint32_t x86_read_cr0() {
    uint32_t r;

    asm volatile("mov %%cr0, %0" : "=r"(r));
    return r;
}

/* Write CR0 */
static inline void x86_write_cr0(uint32_t val) { asm volatile("mov %0, %%cr0" ::"r"(val)); }

/* Write CR3 */
static inline void x86_write_cr3(uint32_t val) { asm volatile("mov %0, %%cr3" ::"r"(val)); }

typedef struct {
    unsigned int present : 1;
    unsigned int rw : 1;
    unsigned int user : 1;
    unsigned int write_through : 1;
    unsigned int cache_disabled : 1;
    unsigned int accessed : 1;
    unsigned int size : 1;
    unsigned int unused : 5;
    unsigned int addr : 20;
} page_tabl_refer_t;

typedef struct {
    unsigned int present : 1;
    unsigned int rw : 1;
    unsigned int user : 1;
    unsigned int write_through : 1;
    unsigned int cache_disabled : 1;
    unsigned int accessed : 1;
    unsigned int dirty : 1;
    unsigned int global : 1;
    unsigned int unused : 4;
    unsigned int addr : 20;
} page_t;

typedef struct { page_tabl_refer_t tabls[1024]; } page_dir_t;

typedef struct { page_t pages[1024]; } page_tabl_t;

void page_init();
#endif
