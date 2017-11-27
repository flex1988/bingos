#include "kernel/console.h"
#include "hal/common.h"
#include "kernel.h"
#include "kernel/vesa.h"
#include "kernel/vga.h"

/* The I/O ports */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5

/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

console_t console;

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

    if (boot_info->framebuffer_type != 1) {
        vga_init(&console);
    } else {
        vesa_init(&console, boot_info);
    }

    console.clear();
}
