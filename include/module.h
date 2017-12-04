#ifndef __MODULE_H__
#define __MODULE_H__

typedef struct {
    char *name;
    int (*init)(void);
    int (*deinit)(void);
} module_t;

extern void *module_load(char *filename);
extern void module_unload(char *name);

#define MODULE_DEFINE(n,i,d) \
    module_t module_info_ ## n = {  \
        .name = #n,     \
        .init = &i,     \
        .deinit = &d    \
    }

void modules_init(void);

#endif
