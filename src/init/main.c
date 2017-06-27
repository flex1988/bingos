#include <stdint.h>

#include "vga/vga.h"

void kmain(char *real_mode,uint32_t stack) {
    vga_init();
    println("vga buffer init.");
    println(real_mode);
    while (1)
        ;
}
