#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "../libc/stdint.h"

// Interrupt descriptor table entry structure
typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

// IDT descriptor
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_descriptor_t;

// Initialize the IDT
void idt_init(void);

// Load the IDT
void idt_load(idt_descriptor_t* idt_desc);

// Set an entry in the IDT
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

// Syscall handler
void syscall_handler(void);

#endif // INTERRUPT_H
