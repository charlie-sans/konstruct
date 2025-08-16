#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdarg.h>

// Console modes
#define CONSOLE_MODE_VGA    0
#define CONSOLE_MODE_SERIAL 1

// Console functions
void console_init(void);
void console_set_mode(int mode);
int console_getchar(void);
void console_putchar(int c);
void console_puts(const char* str);
void console_printf(const char* format, ...);

#endif // CONSOLE_H
