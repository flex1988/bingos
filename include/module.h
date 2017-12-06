#ifndef __MODULE_H__
#define __MODULE_H__

#include <types.h>
#include "multiboot.h"
#include "lib/hashmap.h"

typedef struct {
    char *name;
    int (*init)(void);
    int (*deinit)(void);
} module_t;

typedef struct {
    module_t *mod;
    void *binary;
    hashmap_t *symbols;
    uint32_t end;
    size_t deps_length;
    char *deps;
} module_info_t;

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
