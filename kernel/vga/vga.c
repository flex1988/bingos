#include "vga/vga.h"
#include "types.h"
#include "vga/io.h"

/* The I/O ports */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5

/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

typedef struct vga_char vga_char;

static vga_char* vga_buffer;

static vga_char blank;

static int x;
static int y;

static void move_cursor() {
    uint16_t cursor = y * 80 + x;
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, ((cursor >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, cursor & 0x00FF);
}

static void scroll_screen() {
    if (y >= 25) {
        int i;
        for (i = 0; i < 24 * 80; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }

        for (i = 24 * 80; i < 25 * 80; i++) {
            vga_buffer[i] = blank;
        }

        y = 24;
    }
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
    move_cursor();
}

void printc(char c) {
    switch (c) {
        case '\r':
            x = 0;
        case '\n':
            y++;
            x = 0;

        case '\t':
            do
                printc(' ');
            while (x % 4 != 0);
        default: {
            int location = y * 80 + x;
            vga_buffer[location].ch = c;
            vga_buffer[location].fg = WHITE;
            vga_buffer[location].bg = BLACK;
            y += (x + 1) / 80;
            x = (x + 1) % 80;
        }
    }

    if (x >= 80) {
        x = 0;
        y++;
    }

    move_cursor();
}

void println(char* s) {
    while (*s != '\0') printc(*s++);
    printc('\n');
}
