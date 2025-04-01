# Project Overview

This project consists of three main components: MLibc, an operating system (OS), and a compiler. Each component has its own directory containing source files, headers, and Makefiles for building the respective parts.

## MLibc

MLibc is a lightweight C standard library implementation designed for use in the OS. It provides essential functions for memory management, input/output operations, and string manipulation.

### Structure

- **include/**: Contains header files for MLibc.
  - `memory.h`: Declarations for memory management functions.
  - `stdio.h`: Declarations for input and output functions.
  - `string.h`: Declarations for string manipulation functions.

- **src/**: Contains the implementation of MLibc functions.
  - `memory.c`: Implements memory management functions.
  - `stdio.c`: Implements input and output functions.
  - `string.c`: Implements string manipulation functions.

- **Makefile**: Build instructions for compiling the MLibc library.

## OS

The OS component is a simple operating system that can boot from a floppy disk or UEFI. It utilizes MLibc for its standard library functions.

### Structure

- **src/**: Contains the source files for the OS.
  - `boot.asm`: Bootloader code.
  - `kernel.c`: Kernel code.
  - `bootloader.c`: Bootloader logic.

- **linker.ld**: Linker script defining memory layout and linking options for the OS.

- **uefi_linker.ld**: Linker script for UEFI builds of the OS.

- **Makefile**: Build instructions for compiling the OS.

## Compiler

The Compiler component is designed to generate OS-independent binaries from source code. It includes a lexer, parser, and code generation logic.

### Structure

- **src/**: Contains the source files for the compiler.
  - `lexer.c`: Implements the lexer for tokenizing input source code.
  - `parser.c`: Implements the parser for creating an abstract syntax tree.
  - `codegen.c`: Implements code generation logic.
  - `main.c`: Entry point for the compiler application.

- **include/**: Contains header files for the compiler.
  - `lexer.h`: Declarations for lexer functions.
  - `parser.h`: Declarations for parser functions.
  - `codegen.h`: Declarations for code generation functions.
  - `compiler.h`: General declarations for the compiler.

- **Makefile**: Build instructions for compiling the compiler.

## Building the Project

To build the entire project, navigate to each component's directory (MLibc, OS, Compiler) and run `make`. This will compile the respective components and generate the necessary binaries.

## Running the Project

- For the OS, you can run it using an emulator like QEMU.
- The compiler can be used to compile source files into OS-independent binaries.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests to improve the project.