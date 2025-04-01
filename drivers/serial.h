#ifndef SERIAL_H
#define SERIAL_H
#define SERIAL_COM1_BASE 0x3F8 // COM1 base port
#define SERIAL_COM2_BASE 0x2F8 // COM2 base port
#include "../libc/stdint.h"

// Initialize the serial port
void serial_init(uint16_t com);

// Write a character to the serial port
void serial_write_char(uint16_t com, char c);

// Write a string to the serial port
void serial_write_string(uint16_t com, const char* str);

#endif // SERIAL_H
