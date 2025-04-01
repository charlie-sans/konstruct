#ifndef M_LIBC_STDIO_H
#define M_LIBC_STDIO_H

#include <stddef.h>

void putchar(char c);
void puts(const char *str);
int printf(const char *format, ...);
int scanf(const char *format, ...);
char *gets(char *str);
int getchar(void);

#endif // M_LIBC_STDIO_H