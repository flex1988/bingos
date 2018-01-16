#include "net/net.h"
#include "kernel/process.h"

static int tasklet_pid = 0;

static void net_handler(char *name, void *data) {
    printk("%s", name);
    process_exit(0);
}

void init_netif_funcs() {
    if (!tasklet_pid) {
        tasklet_pid = process_spawn_tasklet(net_handler, "[net]", NULL);
    }
}
