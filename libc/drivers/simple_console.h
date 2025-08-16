#ifndef SIMPLE_CONSOLE_H
#define SIMPLE_CONSOLE_H

#include "../stdarg.h"

// Console modes
#define SIMPLE_CONSOLE_TEXT 0

// Console functions
void simple_console_init(void);
void simple_console_clear(void);
void simple_console_putchar(char c);
void simple_console_print(const char* str);
void simple_console_println(const char* str);
void simple_console_scroll(void);
void simple_console_printf(const char* format, ...);

// External function needed
extern char* itoa(int value, char* str, int base);

#endif // SIMPLE_CONSOLE_H
