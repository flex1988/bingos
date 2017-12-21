#include <stdarg.h>
#include <stddef.h>
#include <types.h>

#include "kernel.h"
#include "kernel/console.h"

static char buf[1024];

extern console_t console;

extern int vsprintf(char *buf, const char *fmt, va_list args);

int printk(const char *fmt, ...) {
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console.println(buf);
    return i;
}

int printc(char c) {
    console.printc(c);
    return 0;
}

void panic(const char *file, uint32_t line, const char *msg) {
    printk("KERNEL PANIC(%s) at %s:%d", msg, file, line);
    for (;;)
        ;
}
