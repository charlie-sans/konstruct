#ifndef LIBC_H
#define LIBC_H

/* Include our custom implementations of standard headers */
#include "stddef.h"
#include "stdint.h"

/* For variadic functions */
typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)

// Memory functions
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* malloc(size_t size);
void free(void* ptr);

// String functions
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* s, int c);
char* strstr(const char* haystack, const char* needle);

// Standard I/O functions
int putchar(int c);
int puts(const char* s);
int printf(const char* format, ...);
char getchar(void);
char* gets(char* str);

// Conversion functions
int atoi(const char* str);
char* itoa(int value, char* str, int base);

#endif /* LIBC_H */
