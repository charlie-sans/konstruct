#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

// Memory allocation functions
void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);

// Memory management utilities
void mem_dump(const void* ptr, size_t size);

#endif // MEMORY_H