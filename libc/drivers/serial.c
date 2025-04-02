#include "serial.h"
#include "../libc/stdint.h"
#include "../libc/libc.h"

// Serial port base addresses
#define SERIAL_COM1_BASE 0x3F8 // COM1 base port
#define SERIAL_COM2_BASE 0x2F8 // COM2 base port

// Serial port registers offsets
#define SERIAL_DATA_PORT(base) (base)
#define SERIAL_INTERRUPT_ENABLE_PORT(base) (base + 1)
#define SERIAL_FIFO_CONTROL_PORT(base) (base + 2)
#define SERIAL_LINE_CONTROL_PORT(base) (base + 3)
#define SERIAL_MODEM_CONTROL_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base) (base + 5)

// Initialize the serial port
void serial_init(uint16_t com) {
    outb(SERIAL_INTERRUPT_ENABLE_PORT(com), 0x00); // Disable all interrupts
    outb(SERIAL_LINE_CONTROL_PORT(com), 0x80);    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_DATA_PORT(com), 0x03);            // Set divisor to 3 (low byte, 38400 baud)
    outb(SERIAL_INTERRUPT_ENABLE_PORT(com), 0x00); // High byte of divisor
    outb(SERIAL_LINE_CONTROL_PORT(com), 0x03);    // 8 bits, no parity, one stop bit
    outb(SERIAL_FIFO_CONTROL_PORT(com), 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_MODEM_CONTROL_PORT(com), 0x0B);   // IRQs enabled, RTS/DSR set
}

// Check if the serial port is ready to transmit
int serial_is_transmit_fifo_empty(uint16_t com) {
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

// Write a character to the serial port
void serial_write_char(uint16_t com, char c) {
    // Handle special characters
    if (c == '\n') {
        // For newline, also send a carriage return for proper terminal behavior
        while (!serial_is_transmit_fifo_empty(com))
            ; // Wait until the transmit FIFO is empty
        outb(SERIAL_DATA_PORT(com), '\r');
        
        while (!serial_is_transmit_fifo_empty(com))
            ; // Wait until the transmit FIFO is empty
        outb(SERIAL_DATA_PORT(com), '\n');
        return;
    }
    
    // For normal characters or other control characters
    while (!serial_is_transmit_fifo_empty(com))
        ; // Wait until the transmit FIFO is empty
    outb(SERIAL_DATA_PORT(com), c);
}

// Write a string to the serial port
void serial_write_string(uint16_t com, const char* str) {
    while (*str) {
        serial_write_char(com, *str++);
    }
}
