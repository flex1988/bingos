#ifndef __MODULE_H__
#define __MODULE_H__

#include <types.h>
#include "multiboot.h"

typedef struct {
    char *name;
    int (*init)(void);
    int (*deinit)(void);
} module_t;

extern void *module_load(void *blob,size_t length);
extern void module_unload(char *name);

#define MODULE_DEFINE(n,i,d) \
    module_t module_info_ ## n = {  \
        .name = #n,     \
        .init = &i,     \
        .deinit = &d    \
    }

void modules_init(multiboot_info_t *boot_info);

#endif
