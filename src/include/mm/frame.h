#ifndef __FRAME_H__
#define __FRAME_H__

#include "multiboot.h"

void frame_init(struct multiboot_info *mbi);
ptr_t get_physaddr(ptr_t virtualaddr);
ptr_t alloc_frame();
void free_frame(ptr_t frame);
int get_first_frame();
void memory_region_init(ptr_t base, uint64_t size);
void memory_region_deinit(ptr_t base, uint64_t size);
#endif
