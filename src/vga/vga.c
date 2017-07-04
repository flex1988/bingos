#include "vga/vga.h"
#include "types.h"

typedef struct vga_char vga_char;

static vga_char* vga_buffer;

static vga_char blank;

static int x;
static int y;

static void update_cursor() {
    // uint16_t cursor = y * 80 + x;
}

void vga_init() {
    vga_buffer = (vga_char*)0xb8000;
    x = 0;
    y = 0;
    blank.ch = ' ';
    blank.fg = BLACK;
    blank.bg = BLACK;
}

void clear_screen() {
    int i;

    for (i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = blank;
    }

    x = 0;
    y = 0;
    update_cursor();
}

void printc(char c) {
    switch (c) {
        case '\r':
            x = 0;
            break;
        case '\n':
            y++;
            x = 0;
            if (y > 80)
                clear_screen();
            break;

        case '\t':
            do
                printc(' ');
            while (x % 4 != 0);
            break;
        default: {
            int location = y * 80 + x;
            vga_buffer[location].ch = c;
            vga_buffer[location].fg = WHITE;
            vga_buffer[location].bg = BLACK;
            y += (x + 1) / 80;
            x = (x + 1) % 80;
        }
    }
}

void println(char* s) {
    while (*s != '\0') printc(*s++);
    printc('\n');
}
