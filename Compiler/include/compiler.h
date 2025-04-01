#ifndef COMPILER_H
#define COMPILER_H

#include <stdint.h>
#include <stdbool.h>

// Function prototypes for the compiler

// Initializes the compiler environment
void init_compiler();

// Compiles the given source code into an intermediate representation
bool compile(const char *source_code);

// Generates assembly code from the intermediate representation
bool generate_assembly(const char *intermediate_code, const char *output_file);

// Cleans up resources used by the compiler
void cleanup_compiler();

#endif // COMPILER_H