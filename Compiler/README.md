# Compiler Documentation

## Overview

The Compiler is designed to generate OS-independent binaries from source code written in a custom language. It utilizes the MLibc library for essential functionalities such as memory management, input/output operations, and string manipulation.

## Features

- **Lexer**: Tokenizes the input source code into manageable tokens.
- **Parser**: Converts tokens into an abstract syntax tree (AST) for further processing.
- **Code Generation**: Transforms the AST into assembly code, which can be assembled into machine code.

## MLibc Integration

The Compiler leverages the MLibc library, which provides a set of standard functions for memory management, input/output, and string manipulation. This allows the Compiler to operate efficiently and effectively without relying on an operating system.

## Building the Compiler

To build the Compiler, navigate to the `Compiler` directory and run the following command:

```bash
make
```

This will compile the source files and generate the executable.

## Usage

After building the Compiler, you can use it to compile source files written in the custom language. The basic usage is as follows:

```bash
./compiler <source_file>
```

Replace `<source_file>` with the path to your source code file.

## Examples

- Compile a simple program:
  ```bash
  ./compiler example.src
  ```

- The output will be an OS-independent binary that can be executed in a suitable environment.

## Contributing

Contributions to the Compiler are welcome! Please feel free to submit issues or pull requests for enhancements and bug fixes.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.