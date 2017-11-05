#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>

int printf(const char *, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
#endif
