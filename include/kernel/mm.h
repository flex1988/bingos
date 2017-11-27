#ifndef __MMAP_H__
#define __MMAP_H__

#include <types.h>

typedef struct vm_area_struct vm_area_t;
struct vm_area_struct {
    uint32_t vm_start;
    uint32_t vm_end;
    vm_area_t *next;
};

void *sys_brk(void *brk);
int do_mmap(uint32_t addr, uint32_t len);
int do_munmap(uint32_t addr, uint32_t len);
void dump_vm_area(vm_area_t *area);
#endif
