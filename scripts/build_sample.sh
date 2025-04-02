#!/bin/bash

# Compile hello.c to hello.bin
echo "Compiling sample program..."

# Create directories if they don't exist
mkdir -p samples

echo "Compiling and linking hello.c..."


# find all .o files in the current directory and subdirectories

# find all .a files in the current directory and subdirectories
ARCHIVES=$(find . -name "*.a" -print | tr '\n' ' ')

# Compile hello.c
gcc -m32 -c -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
    -I"libc" \
    "samples/hello.c" \
    -o "samples/hello.o"

# Link everything together
gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
    -o "samples/hello.bin" \
    "samples/hello.o"  \
    -I"libc" \
    "libc"/*.o \
   
    ./*.o \
    -Wl,-Tuser_linker.ld

echo "Compiled hello.c to samples/hello.bin"
