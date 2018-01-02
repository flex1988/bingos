#ifndef __FRAME_H__
#define __FRAME_H__

#include "kernel/mmu.h"
#include "multiboot.h"

void frame_init(struct multiboot_info *mbi);
ptr_t get_physaddr(ptr_t virtualaddr);
uint32_t alloc_frame();
uint32_t alloc_frames(size_t size);
void free_frame(uint32_t frame);
uint32_t free_pages();
uint32_t get_first_frame();
void get_frame(uint32_t frame);
void memory_region_init(ptr_t base, uint64_t size);
void memory_region_deinit(ptr_t base, uint64_t size);
uint32_t pre_alloc(size_t size, int align, uint32_t *phys);
#endif
