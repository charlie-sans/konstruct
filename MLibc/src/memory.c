#include "libc.h"

// A very simple memory allocator
#define HEAP_SIZE 65536  // 64 KB heap
static uint8_t heap[HEAP_SIZE];
static size_t heap_end = 0;

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    
    return s;
}

// Very simple malloc - just moves a pointer forward
// (no free list, no coalescing, etc.)
void* malloc(size_t size) {
    if (heap_end + size > HEAP_SIZE) {
        // Out of memory
        return NULL;
    }
    
    void* ptr = &heap[heap_end];
    heap_end += size;
    
    // Align to 4 bytes
    heap_end = (heap_end + 3) & ~3;
    
    return ptr;
}

// Our free doesn't actually do anything :)
// In a real implementation, we'd manage a free list
void free(void* ptr) {
    // Not implemented
    (void)ptr;
}
