#ifndef __DRAW_H__
#define __DRAW_H__

#include <types.h>
#include "kernel/vesa.h"

typedef struct color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} color_t;

void graphic_init(vbe_mode_info_structure_t*);
void draw_pixel(int x, int y, color_t* color);
void draw_char(char c, int x, int y, color_t* fg, color_t* bg);
uint32_t frame_buffer_length();
#endif
