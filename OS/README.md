# Operating System Documentation

## Overview

This project is an operating system (OS) designed to run on x86_64 architecture, featuring a custom bootloader and kernel. It also includes a lightweight C standard library, MLibc, which provides essential functionalities for memory management, input/output operations, and string manipulation.

## Features

- **Custom Bootloader**: Written in assembly, the bootloader initializes the system and loads the kernel.
- **Kernel**: The core of the operating system, responsible for managing system resources and providing services to applications.
- **MLibc**: A minimal C library that can be used both within the OS and independently for other projects. It includes:
  - Memory management functions
  - Standard input/output functions
  - String manipulation functions

## Building the OS

To build the operating system, navigate to the `OS` directory and run the following command:

```bash
make
```

This will compile the bootloader, kernel, and create a bootable disk image.

## Running the OS

You can run the OS using QEMU. Use the following command:

```bash
make run-bios
```

This will start the OS in a virtual machine environment.

## Using MLibc

MLibc can be used independently of the OS. To build the MLibc library, navigate to the `MLibc` directory and run:

```bash
make
```

You can then link against MLibc in your own projects by including the headers from the `include` directory and linking the compiled library.

## Contributing

Contributions to the project are welcome. Please feel free to submit issues or pull requests.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.