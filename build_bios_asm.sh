#!/bin/bash

# Navigate to the project directory
cd "$(dirname "$0")"

# Compile just the BIOS assembly file
echo "Compiling BIOS assembly file..."

# Use gcc to compile the S file with the correct flags
gcc -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I. -I.. -g -c -o libc/drivers/bios.o libc/drivers/bios.S

# Check compilation result
if [ $? -eq 0 ]; then
    echo "✓ BIOS assembly file compiled successfully!"
    
    # Examine the object file to see what symbols are exported
    echo "Examining symbols in the object file..."
    nm libc/drivers/bios.o | grep bios_int
else
    echo "✗ Failed to compile BIOS assembly file."
    exit 1
fi

# Try building the wrapper file
echo "Compiling BIOS wrapper file..."
gcc -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I. -Ilibc -g -c -o libc/drivers/bios_wrapper.o libc/drivers/bios_wrapper.c

# Check wrapper compilation result
if [ $? -eq 0 ]; then
    echo "✓ BIOS wrapper file compiled successfully!"
else
    echo "✗ Failed to compile BIOS wrapper file."
    exit 1
fi

echo "Assembly build test completed successfully."
