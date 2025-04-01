#include "libc.h"

// Simple memory allocator

// Memory block header
typedef struct memory_block {
    size_t size;               // Size of this block
    int free;                  // 1 if free, 0 if used
    struct memory_block* next; // Next block in the list
} memory_block_t;

// Initial memory pool size (adjust as needed)
#define MEMORY_POOL_SIZE (1024 * 1024) // 1MB

// Memory pool
static uint8_t memory_pool[MEMORY_POOL_SIZE];

// First block in the memory list
static memory_block_t* first_block = NULL;

// Initialize memory allocator
static void init_memory(void) {
    if (first_block) {
        return; // Already initialized
    }
    
    // Create the first block covering the entire pool
    first_block = (memory_block_t*)memory_pool;
    first_block->size = MEMORY_POOL_SIZE - sizeof(memory_block_t);
    first_block->free = 1;
    first_block->next = NULL;
}

// Allocate memory
void* malloc(size_t size) {
    // Initialize memory if needed
    if (!first_block) {
        init_memory();
    }
    
    // Round size up to multiple of 8 for alignment
    size = (size + 7) & ~7;
    
    // Find a free block of sufficient size
    memory_block_t* block = first_block;
    memory_block_t* prev = NULL;
    
    while (block) {
        if (block->free && block->size >= size) {
            // We found a free block
            
            // Split the block if it's much larger than needed
            if (block->size >= size + sizeof(memory_block_t) + 16) {
                memory_block_t* new_block = (memory_block_t*)((uint8_t*)block + sizeof(memory_block_t) + size);
                new_block->size = block->size - size - sizeof(memory_block_t);
                new_block->free = 1;
                new_block->next = block->next;
                
                block->size = size;
                block->next = new_block;
            }
            
            // Mark the block as used
            block->free = 0;
            
            // Return pointer to the data portion
            return (void*)((uint8_t*)block + sizeof(memory_block_t));
        }
        
        prev = block;
        block = block->next;
    }
    
    // No suitable block found
    return NULL;
}

// Free memory
void free(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // Get the block header
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    // Mark the block as free
    block->free = 1;
    
    // Merge with next block if it's free
    if (block->next && block->next->free) {
        block->size += sizeof(memory_block_t) + block->next->size;
        block->next = block->next->next;
    }
    
    // Find previous block to merge if it's free
    memory_block_t* prev = first_block;
    while (prev && prev->next != block) {
        prev = prev->next;
    }
    
    if (prev && prev->free) {
        prev->size += sizeof(memory_block_t) + block->size;
        prev->next = block->next;
    }
}

// Reallocate memory
void* realloc(void* ptr, size_t size) {
    // Special cases
    if (!ptr) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    // Get the current block header
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    // If new size is smaller, we can just shrink the block
    if (size <= block->size) {
        // Only split if the leftover space is large enough
        if (block->size - size >= sizeof(memory_block_t) + 16) {
            memory_block_t* new_block = (memory_block_t*)((uint8_t*)block + sizeof(memory_block_t) + size);
            new_block->size = block->size - size - sizeof(memory_block_t);
            new_block->free = 1;
            new_block->next = block->next;
            
            block->size = size;
            block->next = new_block;
        }
        return ptr;
    }
    
    // If next block is free and the combined size is enough, merge them
    if (block->next && block->next->free && 
        (block->size + sizeof(memory_block_t) + block->next->size) >= size) {
        
        block->size += sizeof(memory_block_t) + block->next->size;
        block->next = block->next->next;
        
        // Split again if needed
        if (block->size - size >= sizeof(memory_block_t) + 16) {
            memory_block_t* new_block = (memory_block_t*)((uint8_t*)block + sizeof(memory_block_t) + size);
            new_block->size = block->size - size - sizeof(memory_block_t);
            new_block->free = 1;
            new_block->next = block->next;
            
            block->size = size;
            block->next = new_block;
        }
        
        return ptr;
    }
    
    // Otherwise, allocate a new block and copy the data
    void* new_ptr = malloc(size);
    if (!new_ptr) {
        return NULL;
    }
    
    memcpy(new_ptr, ptr, block->size);
    free(ptr);
    
    return new_ptr;
}

// Get memory block size
size_t malloc_usable_size(void* ptr) {
    if (!ptr) {
        return 0;
    }
    
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    return block->size;
}

// Convert memory to a printable string
char* memory_to_str(void* ptr, size_t size, char* buffer, size_t buffer_size) {
    if (!ptr || !buffer || buffer_size == 0) {
        return NULL;
    }
    
    // Convert memory to a hex string
    const uint8_t* mem = (const uint8_t*)ptr;
    size_t pos = 0;
    
    for (size_t i = 0; i < size && pos + 3 < buffer_size; i++) {
        if (i > 0 && pos + 1 < buffer_size) {
            buffer[pos++] = ' ';
        }
        
        // Convert byte to hex
        uint8_t high = (mem[i] >> 4) & 0x0F;
        uint8_t low = mem[i] & 0x0F;
        
        if (pos + 1 < buffer_size) {
            buffer[pos++] = high < 10 ? '0' + high : 'A' + high - 10;
        }
        
        if (pos + 1 < buffer_size) {
            buffer[pos++] = low < 10 ? '0' + low : 'A' + low - 10;
        }
    }
    
    // Null-terminate the string
    buffer[pos] = '\0';
    
    return buffer;
}
