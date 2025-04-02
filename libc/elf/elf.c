#include "elf.h"
#include <libc.h>
#include <fs/fs.h>
#include <syscall.h>

// Buffer for loading the ELF file
#define ELF_BUFFER_SIZE 1048576  // 1MB buffer for ELF files
static char elf_buffer[ELF_BUFFER_SIZE];

// Memory region for program execution
#define PROGRAM_LOAD_ADDRESS 0x400000  // 4MB mark

// Validate ELF header
int elf_validate(const void* elf_data) {
    const elf_header_t* header = (const elf_header_t*)elf_data;
    
    // Check ELF magic number
    if (header->magic != ELF_MAGIC) {
        return 0;  // Not an ELF file
    }
    
    // Check if it's a 32-bit ELF
    if (header->class != 1) {
        return 0;  // Not a 32-bit ELF
    }
    
    // Check if it's an executable
    if (header->type != ET_EXEC) {
        return 0;  // Not an executable
    }
    
    // Check if x86 architecture
    if (header->machine != 3) { // 3 = x86
        return 0;  // Not for x86
    }
    
    return 1;  // Valid ELF file
}

// Load an ELF file into memory
int elf_load(const char* filename) {
    // Get file size
    int file_size = fs_getsize(filename);
    if (file_size <= 0 || file_size > ELF_BUFFER_SIZE) {
        printf("Error: File too large or not found\n");
        return -1;
    }
    
    // Read the file into our buffer
    if (fs_read(filename, elf_buffer, file_size, 0) != file_size) {
        printf("Error: Could not read file\n");
        return -1;
    }
    
    // Validate the ELF header
    if (!elf_validate(elf_buffer)) {
        printf("Error: Invalid ELF file\n");
        return -1;
    }
    
    // Parse the ELF header
    elf_header_t* header = (elf_header_t*)elf_buffer;
    
    // Load program segments
    elf_program_header_t* ph = (elf_program_header_t*)(elf_buffer + header->phoff);
    
    for (int i = 0; i < header->phnum; i++) {
        if (ph[i].type != PT_LOAD) {
            continue;  // Skip non-loadable segments
        }
        
        // Calculate destination address
        char* dest = (char*)(ph[i].vaddr);
        
        // Copy segment from file to memory
        memcpy(dest, elf_buffer + ph[i].offset, ph[i].filesz);
        
        // If memsz > filesz, zero the rest (for .bss)
        if (ph[i].memsz > ph[i].filesz) {
            memset(dest + ph[i].filesz, 0, ph[i].memsz - ph[i].filesz);
        }
    }
    
    return header->entry;  // Return entry point
}

// Environment for running user programs
typedef struct {
    // System call functions
    int (*syscall0)(int num);
    int (*syscall1)(int num, void* arg1);
    int (*syscall2)(int num, void* arg1, void* arg2);
    int (*syscall3)(int num, void* arg1, void* arg2, void* arg3);
    int (*syscall4)(int num, void* arg1, void* arg2, void* arg3, void* arg4);
    
    // libc function pointers
    int (*putchar)(int c);
    int (*getchar)(void);
    int (*puts)(const char* s);
    int (*printf)(const char* format, ...);
    void* (*malloc)(size_t size);
    void (*free)(void* ptr);
    void (*exit)(int status);
} program_env_t;

// Global environment for user programs
static program_env_t program_env = {
    // Syscall functions
    syscall0, syscall1, syscall2, syscall3, syscall4,
    
    // libc functions
    putchar, getchar, puts, printf, malloc, free
};

// Add proper link to scancode_to_ascii function
extern char scancode_to_ascii(unsigned char scancode);

// Execute an ELF binary
int elf_execute(const char* filename, int argc, char** argv) {
    // Load the ELF file
    int entry_point = elf_load(filename);
    if (entry_point < 0) {
        return -1;  // Failed to load
    }
    
    // Create a function pointer to the entry point with environment
    int (*program_entry)(program_env_t* env, int argc, char** argv) = 
        (int(*)(program_env_t*, int, char**))entry_point;
    
    // Call the entry point with the environment
    int result = program_entry(&program_env, argc, argv);
    
    return result;
}
