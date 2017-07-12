#ifndef __IDT_H__
#define __IDT_H__
//****************************************************************************
//**
//**    Idt.h
//**		Interrupt Descriptor Table. The IDT is responsible for providing
//**	the interface for managing interrupts, installing, setting, requesting,
//**	generating, and interrupt callback managing.
//**
//****************************************************************************

#include <stdint.h>

//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================
//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

//! i86 defines 256 possible interrupt handlers (0-255)
#define I86_MAX_INTERRUPTS 256

//! must be in the format 0D110, where D is descriptor type
#define I86_IDT_DESC_BIT16 0x06    // 00000110
#define I86_IDT_DESC_BIT32 0x0E    // 00001110
#define I86_IDT_DESC_RING1 0x40    // 01000000
#define I86_IDT_DESC_RING2 0x20    // 00100000
#define I86_IDT_DESC_RING3 0x60    // 01100000
#define I86_IDT_DESC_PRESENT 0x80  // 10000000

typedef void (*IRQ_HANDLER)(void);

//============================================================================
//    INTERFACE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    INTERFACE STRUCTURES / UTILITY CLASSES
//============================================================================

//! interrupt descriptor
typedef struct {
    //! bits 0-16 of interrupt routine (ir) address
    uint16_t base_low;

    //! code selector in gdt
    uint16_t selector;

    //! reserved, shold be 0
    uint8_t reserved;

    //! bit flags. Set with flags above
    uint8_t flags;

    //! bits 16-32 of ir address
    uint16_t base_high;
} __attribute__((packed)) idt_descriptor_t;

//! returns interrupt descriptor
extern idt_descriptor_t* i86_get_ir(uint32_t i);

//! installs interrupt handler. When INT is fired, it will call this callback
extern void install_interrupt_routine(uint32_t i, uint16_t flags, uint16_t sel, IRQ_HANDLER);

// initialize basic idt
extern void idt_init(uint16_t codeSel);

#endif
