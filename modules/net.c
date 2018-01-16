#include "kernel/process.h"
#include "lib/hashmap.h"
#include "module.h"
#include "kernel.h"

int tasklet_pid = 0;

void net_handler(char *name, void *data) {
    printk("%s", name);
    while(1);
}

void init_netif_funcs() {
    if (!tasklet_pid) {
        tasklet_pid = process_spawn_tasklet(net_handler, "[net]", NULL);
    }
}

int init(void) { return 0; }

int deinit(void) { return 0; }

MODULE_DEFINE(net, init, deinit);
