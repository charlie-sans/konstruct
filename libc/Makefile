# Makefile for building the custom libc as a static library

# Compiler settings
CC = gcc
AR = ar
RANLIB = ranlib

# Compiler flags - match existing project settings
CFLAGS = -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I. -I.. -g
ARFLAGS = rcs

# Source directories
SRC_DIRS = ./

# Source files - all C files in the current directory and subdirectories
C_SRC = $(shell find $(SRC_DIRS) -name '*.c')

# Object files
OBJ = $(C_SRC:.c=.o)

# Output file
LIB = libc.a

# Default target
all: $(LIB)
	echo "lib created with srcs: $(C_SRC)"
	echo "lib created with objs: $(OBJ)"
	echo "lib created with name: $(LIB)"
# Rule to build the library
$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)
	$(RANLIB) $@

# Pattern rule for C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

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

.PHONY: all clean install
