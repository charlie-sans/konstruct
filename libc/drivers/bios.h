#ifndef BIOS_H
#define BIOS_H

#include <stdint.h>

// Structure to hold 16-bit registers for BIOS calls
typedef struct {
    uint16_t di;
    uint16_t si;
    uint16_t bp;
    uint16_t dx;
    uint16_t cx;
    uint16_t bx;
    uint16_t ax;
    uint16_t ds;
    uint16_t es;
    uint16_t flags;
} regs16_t;

// Function declaration for the assembly BIOS interrupt handler
void bios_int(int interrupt_num, regs16_t* regs);

// Helper function to make a VGA BIOS call (interrupt 0x10)
static inline void vga_bios_call(regs16_t* regs) {
    bios_int(0x10, regs);
}

// Helper function to set VGA text mode 3 (80x25)
static inline void set_text_mode(void) {
    regs16_t regs = {0};
    regs.ax = 0x0003;  // AH=0x00 (set video mode), AL=0x03 (text mode 80x25)
    vga_bios_call(&regs);
}

// Helper function to set VGA graphics mode 13h (320x200x256)
static inline void set_graphics_mode(void) {
    regs16_t regs = {0};
    regs.ax = 0x0013;  // AH=0x00 (set video mode), AL=0x13 (graphics mode 320x200)
    vga_bios_call(&regs);
}

#endif // BIOS_H
