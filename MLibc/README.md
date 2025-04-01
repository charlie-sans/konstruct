# MLibc Library

MLibc is a lightweight C standard library designed for use in operating systems and low-level applications. It provides essential functionalities for memory management, input/output operations, and string manipulation.

## Features

- **Memory Management**: Functions for dynamic memory allocation and deallocation.
- **Input/Output Operations**: Basic functions for reading from and writing to the console.
- **String Manipulation**: Functions for handling strings, including copying, concatenation, and comparison.

## Installation

To build the MLibc library, navigate to the `MLibc` directory and run the following command:

```bash
make
```

This will compile the library and generate the necessary object files.

## Usage

To use MLibc in your projects, include the relevant header files in your source code:

```c
#include "MLibc/include/memory.h"
#include "MLibc/include/stdio.h"
#include "MLibc/include/string.h"
```

Link against the compiled library when building your application.

## Examples

### Memory Management Example

```c
#include "MLibc/include/memory.h"

int main() {
    int *arr = (int *)malloc(10 * sizeof(int));
    // Use the allocated memory
    free(arr);
    return 0;
}
```

### Input/Output Example

```c
#include "MLibc/include/stdio.h"

int main() {
    printf("Hello, World!\n");
    return 0;
}
```

### String Manipulation Example

```c
#include "MLibc/include/string.h"

int main() {
    char str1[20] = "Hello, ";
    char str2[] = "World!";
    strcat(str1, str2);
    printf("%s\n", str1);
    return 0;
}
```

## License

This library is open-source and can be used freely in your projects. Contributions are welcome!