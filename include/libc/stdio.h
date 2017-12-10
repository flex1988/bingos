#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>

int printf(const char *, ...);
int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);

#endif
