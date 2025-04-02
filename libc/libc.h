#ifndef LIBC_H
#define LIBC_H

/* Include our custom implementations of standard headers */
#include <stddef.h>
#include <stdint.h>

/* For variadic functions */
typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
// Define constants for video memory and colors
#define VIDEO_MEMORY 0xb8000
#define WHITE_ON_BLACK 0x0f

// Default screen dimensions (80x25 text mode)
#define DEFAULT_WIDTH 80
#define DEFAULT_HEIGHT 25

// VBE mode constants
#define VBE_CONTROLLER_INFO 0x4F00
#define VBE_MODE_INFO 0x4F01
#define VBE_SET_MODE 0x4F02

// Shell buffer size
#define CMD_BUFFER_SIZE 256

// Keyboard port definitions
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Global screen dimensions (can be changed)
// Changed from definitions to declarations with extern
extern int screen_width;
extern int screen_height;
extern int is_graphics_mode;  // 0 for text mode, 1 for graphics mode

// Memory functions
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
size_t malloc_usable_size(void* ptr);
char* memory_to_str(void* ptr, size_t size, char* buffer, size_t buffer_size);

// String functions
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* s, int c);
char* strstr(const char* haystack, const char* needle);
char* strncat(char* dest, const char* src, size_t n);
char* strrchr(const char* s, int c);
char* strdup(const char* str);
char* strtok(char* str, const char* delim);
size_t strspn(const char* str, const char* accept);
size_t strcspn(const char* str, const char* reject);

// Shell program entry point
void shell_main(void);

// Standard I/O functions
int putchar(int c);
int puts(const char* s);
int printf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t size, const char* format, ...);
int vprintf(const char* format, va_list args);
char getchar(void);
char* gets(char* str);

// Conversion functions
int atoi(const char* str);
char* itoa(int value, char* str, int base);

// Math functions
int abs(int x);
double pow(double base, double exp);
double sqrt(double x);

// Keyboard input functions
char scancode_to_ascii(unsigned char scancode);

#endif /* LIBC_H */
