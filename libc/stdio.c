#include <stdarg.h>
#include <stdio.h>
#include <syscall.h>

#define BUFF_SIZE 1024

int printf(const char *fmt, ...) {
    char buf[BUFF_SIZE];
    va_list args;

    if (!fmt)
        return 0;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    syscall_println(buf);
    va_end(args);

    return 0;
}
