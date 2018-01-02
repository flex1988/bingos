#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <types.h>

void kmalloc_init(uint32_t start, uint32_t size);
uint32_t kmalloc(size_t size);
void kfree(void *p);

#endif
