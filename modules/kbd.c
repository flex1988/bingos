#include "fs/fs.h"
#include "hal/isr.h"
#include "kernel/process.h"
#include "module.h"

#define KEY_DEVICE 0x60
#define KEY_PENDING 0x64
#define KEY_IRQ 33

extern process_t *_current_process;

static vfs_node_t *kbd_pipe;

static void keyboard_handler() {
    uint8_t scancode;
    if (inb(KEY_PENDING) & 0x01) {
        scancode = inb(KEY_DEVICE);
        vfs_write(kbd_pipe, 0, 1, (uint8_t[]){scancode});
    }

    irq_ack(KEY_IRQ);
}

int kbd_init(void) {
    printk("Initializing keyboard driver");

    kbd_pipe = create_pipe(128);
    _current_process->fds->entries[0] = kbd_pipe;

    kbd_pipe->flags = VFS_CHARDEVICE;

    vfs_mount("/dev/kbd", kbd_pipe);

    register_interrupt_handler(KEY_IRQ, keyboard_handler);

    return 0;
}

int kbd_deinit(void) { return 0; };

MODULE_DEFINE(kbd, kbd_init, kbd_deinit);
