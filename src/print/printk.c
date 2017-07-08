#include <stdarg.h>
#include <stddef.h>
#include <types.h>

static char buf[1024];

extern int vsprintf(char *buf, const char *fmt, va_list args);

int printk(const char *fmt, ...) {
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    println(buf);
    return i;
}

void panic(const char *file, uint32_t line, const char *msg) {
    printk("KERNEL PANIC(%s) at %s:%d", msg, file, line);
    for (;;)
        ;
}
