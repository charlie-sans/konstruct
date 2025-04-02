# Makefile for building the custom libc as a static library

# Compiler settings
CC = gcc
AS = nasm
AR = ar
RANLIB = ranlib

# Compiler flags - match existing project settings
CFLAGS = -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I. -I.. -g
ASFLAGS = -f elf32
ARFLAGS = rcs

# Source directories
SRC_DIRS = ./

# Source files - all C files in the current directory and subdirectories
C_SRC = $(shell find $(SRC_DIRS) -name '*.c')
# Assembly source files - be specific about extensions and use correct capitalization
ASM_S_SRC = $(shell find $(SRC_DIRS) -name '*.S' -o -name '*.s')
ASM_ASM_SRC = $(shell find $(SRC_DIRS) -name '*.asm')

# Object files
C_OBJ = $(C_SRC:.c=.o)
ASM_S_OBJ = $(ASM_S_SRC:.S=.o)
ASM_ASM_OBJ = $(ASM_ASM_SRC:.asm=.o)
OBJ = $(C_OBJ) $(ASM_S_OBJ) $(ASM_ASM_OBJ)

# Output file
LIB = libc.a

# Default target
all: $(LIB)
	@echo "lib created with C srcs: $(C_SRC)"
	@echo "lib created with ASM srcs: $(ASM_S_SRC) $(ASM_ASM_SRC)"
	@echo "lib created with objs: $(OBJ)"
	@echo "lib created with name: $(LIB)"

# Rule to build the library
$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)
	$(RANLIB) $@

# Pattern rule for C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for GNU Assembly files with uppercase .S extension (with CPP preprocessing)
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for GNU Assembly files with lowercase .s extension (no preprocessing)
%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for NASM assembly files
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Create subdirectories for object files if needed
$(shell mkdir -p elf)

# Install rule - copies the library to a location for linking
install: $(LIB)
	mkdir -p ../lib
	cp $(LIB) ../lib/

# Clean rule
clean:
	rm -f $(OBJ) $(LIB)
	rm -f ../lib/$(LIB)

# Debug target to print variables
debug:
	@echo "C_SRC = $(C_SRC)"
	@echo "ASM_S_SRC = $(ASM_S_SRC)"
	@echo "ASM_ASM_SRC = $(ASM_ASM_SRC)"
	@echo "C_OBJ = $(C_OBJ)"
	@echo "ASM_S_OBJ = $(ASM_S_OBJ)"
	@echo "ASM_ASM_OBJ = $(ASM_ASM_OBJ)"
	@echo "OBJ = $(OBJ)"

.PHONY: all clean install debug
