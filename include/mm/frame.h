#ifndef __FRAME_H__
#define __FRAME_H__

#include "mm/mmu.h"
#include "multiboot.h"

void frame_init(struct multiboot_info *mbi);
ptr_t get_physaddr(ptr_t virtualaddr);
uint32_t alloc_frame();
uint32_t alloc_frames(size_t size);
void free_frame(ptr_t frame);
uint32_t get_first_frame();
void memory_region_init(ptr_t base, uint64_t size);
void memory_region_deinit(ptr_t base, uint64_t size);
void page_map(page_t *page, int kernel, int rw);
void page_identical_map(page_t *page, int kernel, int rw, uint32_t virt);
#endif
