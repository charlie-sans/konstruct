#include <stdio.h>
#include "compiler.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    const char *source_file = argv[1];

    // Initialize the compiler
    Compiler compiler;
    if (!compiler_init(&compiler, source_file)) {
        fprintf(stderr, "Failed to initialize compiler.\n");
        return 1;
    }

    // Run the compilation process
    if (!compiler_run(&compiler)) {
        fprintf(stderr, "Compilation failed.\n");
        compiler_cleanup(&compiler);
        return 1;
    }

    // Cleanup and exit
    compiler_cleanup(&compiler);
    return 0;
}