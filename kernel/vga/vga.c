#include "kernel/vga.h"
#include "hal/common.h"
#include "kernel.h"
#include "types.h"

/* The I/O ports */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5

/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

typedef struct vga_char vga_char;

static vga_char* vga_buffer;

static vga_char blank;

static uint8_t cursor_x;
static uint8_t cursor_y;

static void move_cursor() {
    uint16_t cursor = cursor_y * 80 + cursor_x;
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, ((cursor >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, cursor & 0x00FF);
}

static void scroll_screen() {
    if (cursor_y >= 25) {
        int i;
        for (i = 0; i < 24 * 80; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }

        for (i = 24 * 80; i < 25 * 80; i++) {
            vga_buffer[i] = blank;
        }

        cursor_y = 24;
    }
}

void vga_init() {
    vga_buffer = (vga_char*)0xb8000;
    cursor_x = 0;
    cursor_y = 0;

    uint8_t attributeByte = (0 << 4) | (15 & 0x0F);
    uint16_t v = 0x20 | (attributeByte << 8);
    memcpy(blank, v, sizeof(uint16_t));
}

void clear_screen() {
    int i;

    for (i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = blank;
    }

    cursor_x = 0;
    cursor_y = 0;
    move_cursor();
}

void printc(char c) {
    uint8_t back = 0;
    uint8_t fore = 15;

    uint8_t attribute_byte = (back << 4) | (fore & 0x0F);
    uint16_t attribute = attribute_byte << 8;
    uint16_t* location;

    if (c == 0x08 && cursor_x) {
        cursor_x--;
    }

    else if (c == 0x09) {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    }

    else if (c == '\r') {
        cursor_x = 0;
    }

    else if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    }

    else if (c >= ' ') {
        location = vga_buffer + (cursor_y * 80 + cursor_x);
        *location = c | attribute;
        cursor_x++;
    }

    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }

    scroll_screen();
    move_cursor();
}

void println(char* s) {
    while (*s != '\0') printc(*s++);
    printc('\n');
}
