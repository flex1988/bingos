#include <stdint.h>

#include "print/printk.h"

void kmain(char *real_mode,uint32_t stack) {
    vga_init();
    printk("vga buffer init.\n");
    while (1)
        ;
}
