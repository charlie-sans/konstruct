#!/bin/bash

# Compilation script for user programs

# Set paths
#USERLIBC_DIR="$(pwd)/libc"
#SYSCALL_DIR="$(pwd)/syscalls"

# Get the source file from command line
if [ $# -lt 1 ]; then
    echo "Usage: $0 <source_file.c>"
    exit 1
fi

SOURCE=$1
OUTFILE="${SOURCE%.*}"

# Compile the program
gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
    -I"libc/"  \
    -o "$OUTFILE" "$SOURCE" "libc"/*.c \
    -Wl,-Tuser_linker.ld

echo "Compiled $SOURCE to $OUTFILE"
