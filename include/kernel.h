#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "types.h"

#define __KERNEL__ 1

#define PANIC(msg) panic(__FILE__, __LINE__, msg)

#define ASSERT(b) ((b) ? (void)0 : panic(__FILE__, __LINE__, #b))

extern void panic(const char *file, uint32_t line, const char *msg);

#define NULL 0

#define true 1
#define false 0

extern void interrupt_enable(void);
extern void interrupt_disable(void);
extern void interrupt_resume(void);

#define IRQ_ON interrupt_enable()
#define IRQ_OFF interrupt_disable()
#define IRQ_RES interrupt_resume()

#endif
