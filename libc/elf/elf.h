#ifndef ELF_H
#define ELF_H

#include "../libc/stdint.h"

// ELF Magic Numbers
#define ELF_MAGIC 0x464C457F  // '\x7FELF' in little endian

// ELF file header
typedef struct {
    uint32_t magic;           // Must be 0x7F 'E' 'L' 'F'
    uint8_t  class;           // 1 = 32bit, 2 = 64bit
    uint8_t  endianness;      // 1 = little, 2 = big
    uint8_t  version;         // Should be 1
    uint8_t  abi;             // Target ABI
    uint8_t  abi_version;     // ABI version
    uint8_t  padding[7];      // Unused
    uint16_t type;            // Object file type
    uint16_t machine;         // Target machine
    uint32_t e_version;       // Object file version
    uint32_t entry;           // Entry point virtual address
    uint32_t phoff;           // Program header table offset
    uint32_t shoff;           // Section header table offset
    uint32_t flags;           // Processor-specific flags
    uint16_t ehsize;          // ELF header size
    uint16_t phentsize;       // Program header entry size
    uint16_t phnum;           // Program header entry count
    uint16_t shentsize;       // Section header entry size
    uint16_t shnum;           // Section header entry count
    uint16_t shstrndx;        // Section header string table index
} elf_header_t;

// ELF program header
typedef struct {
    uint32_t type;            // Segment type
    uint32_t offset;          // Segment offset in file
    uint32_t vaddr;           // Segment virtual address
    uint32_t paddr;           // Segment physical address
    uint32_t filesz;          // Segment size in file
    uint32_t memsz;           // Segment size in memory
    uint32_t flags;           // Segment flags
    uint32_t align;           // Segment alignment
} elf_program_header_t;

// ELF section header
typedef struct {
    uint32_t name;            // Section name offset
    uint32_t type;            // Section type
    uint32_t flags;           // Section flags
    uint32_t addr;            // Section virtual address
    uint32_t offset;          // Section offset in file
    uint32_t size;            // Section size
    uint32_t link;            // Section header table link
    uint32_t info;            // Extra information
    uint32_t addralign;       // Section alignment
    uint32_t entsize;         // Entry size if section has table
} elf_section_header_t;

// ELF types
#define ET_NONE   0           // No file type
#define ET_REL    1           // Relocatable file
#define ET_EXEC   2           // Executable file
#define ET_DYN    3           // Shared object file
#define ET_CORE   4           // Core file

// ELF program header types
#define PT_NULL    0          // Unused
#define PT_LOAD    1          // Loadable segment
#define PT_DYNAMIC 2          // Dynamic linking info
#define PT_INTERP  3          // Interpreter
#define PT_NOTE    4          // Auxiliary info
#define PT_SHLIB   5          // Reserved
#define PT_PHDR    6          // Program header table

// Function declarations
int elf_validate(const void* elf_data);
int elf_load(const char* filename);
int elf_execute(const char* filename, int argc, char** argv);

#endif // ELF_H
