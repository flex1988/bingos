#include "vga.h"

void kernel_main() {
    vga_init();
    println("vga buffer init.");
    while (1)
        ;
}
