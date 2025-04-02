#!/bin/bash

# Compile hello.c to hello.bin
echo "Compiling sample program..."

# Create directories if they don't exist
mkdir -p samples

echo "Compiling and linking hello.c..."


# find all .o files in the current directory and subdirectories



# Compile hello.c
gcc -m32 -c -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
    -Ilibc -o samples/hello.o samples/hello.c

# Link with the static library
gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
    -o "samples/hello.bin" \
    "samples/hello.o" \
    -Llibc -lc \
    -Wl,-Tuser_linker.ld

echo "Compiled hello.c to samples/hello.bin"
