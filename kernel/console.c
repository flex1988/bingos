#include "kernel/console.h"
#include "hal/common.h"
#include "kernel.h"
#include "kernel/vesa.h"
#include "kernel/vga.h"

console_t console;

static void serial_clear_screen()
{
    ;
}

static void serial_printc(char c)
{
    outb(0x3F8, c);
}

static void serial_println(char* s)
{
    while (*s != '\0') serial_printc(*s++);
    serial_printc('\n');
}

void tty_init(multiboot_info_t *boot_info) {
    if (!(boot_info->flags & (1 << 11))) {
        return;
    }

    vbe_info_structure_t *info = (vbe_info_structure_t *)boot_info->vbe_control_info;
    if (strcmp(info->signature, "VESA") != 0) {
        return;
    }

    if (!(boot_info->flags & (1 << 12))) {
        ;  // printk("frame buffer is not available");
    }

    vbe_mode_info_structure_t *mode_info = (vbe_mode_info_structure_t *)boot_info->vbe_mode_info;

    if (0) {
        console.println = serial_println;
        console.printc = serial_printc;
        console.clear = serial_clear_screen;
    } else if (boot_info->framebuffer_type != 1) {
        vga_init(&console);
    } else {
        vesa_init(&console, boot_info);
    }

    console.clear();
}
