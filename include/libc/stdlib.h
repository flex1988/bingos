#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <types.h>

extern void *malloc(size_t size);
extern void free(void *p);
extern void *realloc(void *ptr, size_t size);
extern void *calloc(size_t nelem, size_t elsize);
int execvp(const char *file, char **argv);
int atoi(const char *s);

#endif
