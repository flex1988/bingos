#include "vga.h"

void kernel_main() {
    vga_init();
    println("hello world!");
    while (1)
        ;
}
