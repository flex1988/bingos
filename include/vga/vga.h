#ifndef __VGA_H__
#define __VGA_H__
typedef enum {
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    L_GREY,
    D_GREY,
    L_BLUE,
    L_GREEN,
    L_CYAN,
    L_RED,
    L_MAG,
    L_BROWN,
    WHITE
} vga_color;

struct vga_char {
    char ch : 8;
    char fg : 4;
    char bg : 4;
};

void println(char *msg);
void printc(char c);
void vga_init();
void clear_screen();
#endif
