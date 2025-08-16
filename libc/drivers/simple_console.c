#include "../libc.h"
#include "../stdarg.h"
#include "simple_console.h"

// Simple console implementation that uses VGA text mode
static int console_mode = SIMPLE_CONSOLE_TEXT;
static int cursor_x = 0;
static int cursor_y = 0;

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Simple console initialization
void simple_console_init(void) {
    console_mode = SIMPLE_CONSOLE_TEXT;
    cursor_x = 0;
    cursor_y = 0;
    
    // Clear screen
    simple_console_clear();
}

// Clear the screen
void simple_console_clear(void) {
    char* video_memory = (char*)VGA_MEMORY;
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i * 2] = ' ';      // Character
        video_memory[i * 2 + 1] = 0x07; // Attribute (white on black)
    }
    
    cursor_x = 0;
    cursor_y = 0;
}

// Put a character at the current cursor position
void simple_console_putchar(char c) {
    char* video_memory = (char*)VGA_MEMORY;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            int offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;
            video_memory[offset] = ' ';
            video_memory[offset + 1] = 0x07;
        }
    } else if (c >= 32) { // Printable characters
        int offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;
        video_memory[offset] = c;
        video_memory[offset + 1] = 0x07;
        cursor_x++;
    }
    
    // Handle line wrapping
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    // Handle scrolling
    if (cursor_y >= VGA_HEIGHT) {
        simple_console_scroll();
        cursor_y = VGA_HEIGHT - 1;
    }
}

// Print a string
void simple_console_print(const char* str) {
    while (*str) {
        simple_console_putchar(*str);
        str++;
    }
}

// Print a string with newline
void simple_console_println(const char* str) {
    simple_console_print(str);
    simple_console_putchar('\n');
}

// Scroll the screen up by one line
void simple_console_scroll(void) {
    char* video_memory = (char*)VGA_MEMORY;
    
    // Move all lines up by one
    for (int line = 1; line < VGA_HEIGHT; line++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            int src_offset = (line * VGA_WIDTH + col) * 2;
            int dst_offset = ((line - 1) * VGA_WIDTH + col) * 2;
            
            video_memory[dst_offset] = video_memory[src_offset];
            video_memory[dst_offset + 1] = video_memory[src_offset + 1];
        }
    }
    
    // Clear the bottom line
    for (int col = 0; col < VGA_WIDTH; col++) {
        int offset = ((VGA_HEIGHT - 1) * VGA_WIDTH + col) * 2;
        video_memory[offset] = ' ';
        video_memory[offset + 1] = 0x07;
    }
}

// Simple printf implementation
void simple_console_printf(const char* format, ...) {
    // Simple implementation - just handle %s, %d, %c, %x
    const char* p = format;
    va_list args;
    va_start(args, format);
    
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++; // Skip '%'
            switch (*p) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    if (s) simple_console_print(s);
                    break;
                }
                case 'd': {
                    int n = va_arg(args, int);
                    char buf[12];
                    itoa(n, buf, 10);
                    simple_console_print(buf);
                    break;
                }
                case 'x': {
                    int n = va_arg(args, int);
                    char buf[12];
                    itoa(n, buf, 16);
                    simple_console_print(buf);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    simple_console_putchar(c);
                    break;
                }
                case '%':
                    simple_console_putchar('%');
                    break;
                default:
                    simple_console_putchar('%');
                    simple_console_putchar(*p);
                    break;
            }
        } else {
            simple_console_putchar(*p);
        }
        p++;
    }
    
    va_end(args);
}
