#include "debug.h"
#include "libc/libc.h"

// Panic function - prints error and halts
void panic(const char* message) {
    printf("\n\n*** KERNEL PANIC ***\n");
    printf("Error: %s\n", message);
    printf("System halted. Press Ctrl+C in QEMU to exit.\n");
    printf("For debugging: qemu-system-i386 -cdrom konstruct.iso -s -S\n");
    printf("Then: gdb -ex 'target remote localhost:1234'\n");
    
    // Disable interrupts and halt
    __asm__ volatile("cli");
    while (1) {
        __asm__ volatile("hlt");
    }
}

// Debug breakpoint
void debug_break(void) {
    // This will trigger a debugger if attached
    __asm__ volatile("int $0x03");
}

// Print keyboard debugging information
void debug_keyboard_state(void) {
    printf("\n=== KEYBOARD DEBUG INFO ===\n");
    
    // Read keyboard status port
    unsigned char status = inb(KEYBOARD_STATUS_PORT);
    printf("Keyboard Status Port (0x64): 0x%02X\n", status);
    printf("  Bit 0 (Output Buffer Full): %s\n", (status & 0x01) ? "YES" : "NO");
    printf("  Bit 1 (Input Buffer Full):  %s\n", (status & 0x02) ? "YES" : "NO");
    
    // If there's data available, read it
    if (status & 0x01) {
        unsigned char data = inb(KEYBOARD_DATA_PORT);
        printf("Keyboard Data Port (0x60): 0x%02X (scancode)\n", data);
    } else {
        printf("Keyboard Data Port (0x60): No data available\n");
    }
    
    printf("===========================\n\n");
}
