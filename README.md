# konstruct - A Simple Operating System

![konstruct Logo](docs/images/logo.png)

## Overview

konstruct is a simple, educational operating system designed to demonstrate basic OS concepts including:

- Custom bootloader with GRUB integration
- 32-bit protected mode kernel
- Custom C library implementation (libc)
- Basic graphics support (text mode and VGA)
- Simple filesystem implementation
- Program execution capabilities

## Table of Contents

- [Prerequisites](#prerequisites)
- [Building](#building)
- [Running](#running)
- [Project Structure](#project-structure)
- [Features](#features)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

## Prerequisites

To build and run konstruct, you need:

- GNU/Linux development environment (or WSL on Windows)
- GCC cross-compiler for i386 target
- NASM assembler
- GNU Make
- GRUB bootloader utilities
- QEMU for emulation

### Setting up the Build Environment

```bash
# For Debian/Ubuntu based systems
sudo apt update
sudo apt install build-essential nasm xorriso grub-pc-bin qemu-system-x86
```

## Building

To build the entire project:

```bash
./build_all.sh
```

This will:
1. Compile the kernel and libc
2. Create a bootable ISO image

For testing only the BIOS assembly code:

```bash
./test_build_bios.sh
```

## Running

To run the OS in QEMU:

```bash
qemu-system-i386 -cdrom build/konstruct.iso
```

For debugging with GDB:

```bash
# Terminal 1
qemu-system-i386 -cdrom build/konstruct.iso -s -S

# Terminal 2
gdb -ex "target remote localhost:1234" -ex "symbol-file build/kernel.bin"
```

## Project Structure

```
/
├── build/              # Build output directory
├── docs/               # Documentation
├── fs/                 # Filesystem implementation
├── grub/               # GRUB bootloader configuration
├── kernel.c            # Kernel main file
├── libc/               # Custom C library implementation
│   ├── drivers/        # Device drivers
│   ├── elf/            # ELF binary format handling
│   ├── fs/             # Filesystem interfaces
│   ├── stdio.c         # Standard I/O implementation
│   ├── stdlib.c        # Standard library functions
│   ├── string.c        # String manipulation functions
│   └── ...
├── samples/            # Sample user programs
└── syscalls/           # System call implementation
```

## Features

### Kernel

- Multiboot compliant for GRUB integration
- Basic memory management
- Interrupt handling
- System call interface

### libc Implementation

- Standard C library functions (stdio, stdlib, string)
- Custom implementations for a freestanding environment
- See [LIBC.md](docs/LIBC.md) for full details

### Filesystem

- Basic filesystem with directories and files
- Support for reading and writing files
- Mount system for different devices
- See [FILESYSTEM.md](docs/FILESYSTEM.md) for more details

### Graphics

- Text mode (80x25) support
- VGA graphics mode (320x200)
- Support for higher resolutions via VESA BIOS Extensions
- Custom font rendering from BMP files
- See [GRAPHICS.md](docs/GRAPHICS.md) for details

### Shell

- Basic command-line interface
- Commands for file operations, system control, and application execution

## Documentation

- [LIBC Documentation](docs/LIBC.md)
- [Filesystem Documentation](docs/FILESYSTEM.md)
- [Graphics Documentation](docs/GRAPHICS.md)
- [Contributing Guidelines](CONTRIBUTING.md)

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
