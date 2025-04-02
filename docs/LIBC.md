# konstruct C Library (libc) Documentation

This document describes the custom C library implementation included with konstruct. The library provides standard C functions in a freestanding environment, allowing applications to be written using familiar interfaces.

## Table of Contents

- [Overview](#overview)
- [Supported Headers](#supported-headers)
- [Building the Library](#building-the-library)
- [Key Components](#key-components)
- [Standard Functions](#standard-functions)
- [System Calls](#system-calls)
- [Adding New Functions](#adding-new-functions)

## Overview

The konstruct C library is a minimal implementation of standard C library functions that can operate without an underlying operating system. It provides the essential functionality needed for application development while maintaining compatibility with standard C where possible.

## Supported Headers

The library implements versions of the following standard headers:

- `<stddef.h>` - Basic type definitions
- `<stdint.h>` - Integer types with defined sizes
- `<stdio.h>` - Standard I/O functions
- `<stdlib.h>` - General utilities
- `<string.h>` - String manipulation
- `<unistd.h>` - POSIX API functions
- `<sys/types.h>` - System types
- `<sys/stat.h>` - File status functions

## Building the Library

The library is built using the makefile in the `libc` directory:

```bash
cd libc
make
```

This produces a static library `libc.a` that can be linked with applications.

## Key Components

### Standard I/O (`stdio.c`)

Implements:
- Character I/O: `getchar()`, `putchar()`
- String I/O: `puts()`, `gets()`
- Formatted I/O: `printf()`, `sprintf()`, `snprintf()`
- Buffer management for screen output

### Standard Library (`stdlib.c`)

Implements:
- Memory allocation: `malloc()`, `free()`, `realloc()`
- String conversion: `atoi()`, `itoa()`
- Random numbers (simple implementation)
- System functions: `reboot()`, `soft_reboot()`
- Command handling for shell integration

### String Manipulation (`string.c`)

Implements:
- String functions: `strlen()`, `strcpy()`, `strcmp()`, etc.
- Memory functions: `memcpy()`, `memset()`, `memmove()`, etc.

### Memory Management (`memory.c`)

Implements:
- Basic heap allocation system
- Tracks allocated and free blocks

## Standard Functions

### stdio.h Functions

| Function | Description |
|----------|-------------|
| `int putchar(int c)` | Outputs a character to the screen |
| `int getchar(void)` | Reads a character from keyboard input |
| `int puts(const char* s)` | Outputs a string followed by newline |
| `char* gets(char* s)` | Reads a line of input (unsafe, provided for compatibility) |
| `int printf(const char* format, ...)` | Prints formatted output to screen |
| `int sprintf(char* str, const char* format, ...)` | Writes formatted output to string |
| `int snprintf(char* str, size_t size, const char* format, ...)` | Writes formatted output with size limit |

### stdlib.h Functions

| Function | Description |
|----------|-------------|
| `void* malloc(size_t size)` | Allocates memory from heap |
| `void free(void* ptr)` | Releases allocated memory |
| `void* realloc(void* ptr, size_t size)` | Resizes allocated memory block |
| `int atoi(const char* str)` | Converts string to integer |
| `char* itoa(int value, char* str, int base)` | Converts integer to string with specified base |
| `void exit(int status)` | Terminates process with status code |

### string.h Functions

| Function | Description |
|----------|-------------|
| `size_t strlen(const char* str)` | Returns string length |
| `char* strcpy(char* dest, const char* src)` | Copies string |
| `char* strncpy(char* dest, const char* src, size_t n)` | Copies with length limit |
| `int strcmp(const char* s1, const char* s2)` | Compares strings |
| `void* memcpy(void* dest, const void* src, size_t n)` | Copies memory |
| `void* memset(void* s, int c, size_t n)` | Fills memory with byte value |
| `void* memmove(void* dest, const void* src, size_t n)` | Copies memory (handles overlap) |

## System Calls

The library interfaces with the kernel through system calls. These are implemented in the `syscalls` directory:

| System Call | Description |
|-------------|-------------|
| `_read` | Reads from file descriptor |
| `_write` | Writes to file descriptor |
| `_open` | Opens a file |
| `_close` | Closes a file descriptor |
| `_fstat` | Gets file status |
| `_lseek` | Repositions read/write file offset |
| `_sbrk` | Increases program data space |
| `_exit` | Terminates the process |

## Adding New Functions

To add a new function to the library:

1. Identify the appropriate source file (or create a new one if needed)
2. Implement the function following the coding style guidelines
3. Add the function prototype to the appropriate header file
4. Update the Makefile if you added a new source file
5. Rebuild the library

### Example: Adding a new string function

```c
// In string.c
char* strtoupper(char* str) {
    char* p = str;
    while (*p) {
        if (*p >= 'a' && *p <= 'z') {
            *p = *p - 'a' + 'A';
        }
        p++;
    }
    return str;
}

// In libc.h
char* strtoupper(char* str);
```

Then rebuild the library with `make` in the libc directory.
