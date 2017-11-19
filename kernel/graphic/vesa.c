#include "kernel/vesa.h"
#include "kernel.h"
#include "kernel/console.h"
#include "kernel/drawing.h"
#include "multiboot.h"

#include <types.h>

static uint8_t cursor_x;
static uint8_t cursor_y;

static uint8_t char_height;
static uint8_t char_width;

extern color_t RED;
extern color_t BLACK;

static int console_height;
static int console_width;

static void vesa_printc(char c);
static void vesa_println(char *msg);
static void vesa_scroll_screen();
static void vesa_move_cursor();
static void vesa_clear_screen();

void vesa_init(console_t *console, multiboot_info_t *boot_info) {
    if (!(boot_info->flags & (1 << 11))) {
        return;
    }

    vbe_info_structure_t *info = (vbe_info_structure_t *)boot_info->vbe_control_info;
    if (strcmp(info->signature, "VESA") != 0) {
        return;
    }

    ASSERT(boot_info->flags & (1 << 12));

    vbe_mode_info_structure_t *mode_info = (vbe_mode_info_structure_t *)boot_info->vbe_mode_info;

    ASSERT(boot_info->framebuffer_type == 1);

    graphic_init(mode_info);

    char_height = 16;
    char_width = 8;

    console_height = mode_info->height / char_height - 1;
    console_width = mode_info->width / char_width + 1;

    cursor_x = cursor_y = 0;

    console->println = vesa_println;
    console->printc = vesa_printc;
    console->clear = vesa_clear_screen;
}

static void vesa_printc(char c) {
    uint8_t back = 0;
    uint8_t fore = 15;

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
        draw_char(c, cursor_x * char_width, cursor_y * char_height, &RED, &BLACK);
        cursor_x++;
    }

    if (cursor_x >= console_width) {
        cursor_x = 0;
        cursor_y++;
    }

    vesa_scroll_screen();
    vesa_move_cursor();
}

static void vesa_println(char *s) {
    while (*s != '\0') vesa_printc(*s++);
    vesa_printc('\n');
}

static void vesa_scroll_screen() {}
static void vesa_move_cursor() {}
static void vesa_clear_screen() {}
