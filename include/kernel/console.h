#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <types.h>

#include "multiboot.h"

void tty_init(multiboot_info_t *boot_info);

typedef void (*println_t)(char *);
typedef void (*printc_t)(char c);
typedef void (*clear_t)(void);

typedef struct console {
    println_t println;
    printc_t printc;
    clear_t clear;
} console_t;

#endif
