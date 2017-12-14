#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int printf(const char *, ...);
int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);

#endif
