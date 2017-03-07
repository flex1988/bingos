#include "vga.h"

static struct vga_char* vga_buffer;

static int x;
static int y;

void vga_init() {
    vga_buffer = (struct vga_char*)0xb8000;
    x = 0;
    y = 0;
}

void printc(char c) {
    switch (c) {
        case '\r':
            x = 0;
            break;
        case '\n':
            y++;
            x = 0;
            break;

        case '\t':
            do
                printc(' ');
            while (x % 4 != 0);
            break;
        default: {
            int location = y * 80 + x;
            vga_buffer[location].ch = c;
            vga_buffer[location].fg = RED;
            vga_buffer[location].bg = BLACK;
            y += (x + 1) / 80;
            x = (x + 1) % 80;
        }
    }
}

void println(char* s) {
    while (*s != '\0') printc(*s++);
}
