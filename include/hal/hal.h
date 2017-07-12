#ifndef __HAL_H__
#define __HAL_H__

typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_descriptor_t;

extern void hal_init();
extern void hal_exit();
#endif
