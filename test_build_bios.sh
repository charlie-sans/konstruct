#!/bin/bash

# Compile just the BIOS assembly file to verify it works
echo "Compiling BIOS assembly file..."

# Compile the BIOS.S file
gcc -m32 -c -o libc/drivers/bios.o libc/drivers/bios.S

# Check compilation result
if [ $? -eq 0 ]; then
    echo "BIOS assembly file compiled successfully!"
else
    echo "Failed to compile BIOS assembly file."
    exit 1
fi

echo "Assembly object file created at libc/drivers/bios.o"
