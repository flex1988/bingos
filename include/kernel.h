#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <types.h>

#define PANIC(msg) panic(__FILE__, __LINE__, msg)

#define ASSERT(b) ((b) ? (void)0 : panic(__FILE__, __LINE__, #b))

extern void panic(const char *file, uint32_t line, const char *msg);

#define NULL 0

#define true 1
#define false 0

#endif
