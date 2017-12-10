#ifndef __COMMON_H__
#define __COMMON_H__

#include <types.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t ins(uint16_t port);
void outs(uint16_t port, uint16_t data);
void outl(uint16_t port, uint32_t data);
void insm(uint16_t port, uint8_t *data, uint32_t size);

#endif
