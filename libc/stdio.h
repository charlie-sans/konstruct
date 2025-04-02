#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <libc.h>

// Character input and output functions
int putchar(int c);
char getchar(void);
int puts(const char* s);
char* gets(char* str);

// Formatted output functions
int printf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t size, const char* format, ...);
int vsnprintf(const char* format, va_list args);

// String conversion functions
int atoi(const char* str);
char* itoa(int value, char* str, int base);

// Keyboard input related functions
char scancode_to_ascii(unsigned char scancode);
extern unsigned char read_scan_code(void);

// Low-level display functions
void print_char(char c);

// Define constants used by the implementation
#ifndef VIDEO_MEMORY
#define VIDEO_MEMORY 0xB8000
#endif

#ifndef WHITE_ON_BLACK
#define WHITE_ON_BLACK 0x07
#endif


#endif /* _STDIO_H */
