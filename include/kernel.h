#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <types.h>

#define PANIC(msg) panic(__FILE__, __LINE__, msg)

extern void panic(const char *file, uint32_t line, const char *msg);

extern ptr_t kmalloc(size_t size);

extern void outb(uint16_t port, uint8_t value);
#endif
