#ifndef __PAGE_H__
#define __PAGE_H__

#define PAGE_SIZE (4096)

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

extern void page_init();
void enable_paging();
#endif
