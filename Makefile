# Makefile for building our OS with GRUB bootloader

# Compiler settings
CC = gcc
AS = nasm
LD = ld

# Detect macOS and set cross-compiler flags if needed
UNAME_S := $(shell uname -s)
# ifeq ($(UNAME_S),Darwin)
#     # On macOS, we need to use a cross-compiler for i386
#     CC = i386-elf-gcc
#     LD = i386-elf-ld
#     # If homebrew cross-compiler is not installed, show helpful message
#     ifeq ($(shell which $(CC)),)
#         $(warning "i386-elf-gcc not found. Please install using:")
#         $(warning "brew tap nativeos/i386-elf-toolchain")
#         $(warning "brew install i386-elf-binutils i386-elf-gcc")
#     endif
# endif

# Flags for kernel build
CFLAGS = -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -I. -g
CFLAGS += -I. -Id:\tempsting\newlib\include
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -Tlinker.ld


# Add Newlib source directories
NEWLIB_SRC = d:\tempsting\newlib
CFLAGS += -I$(NEWLIB_SRC)\include

# Add Newlib source files
NEWLIB_OBJS = \
    $(NEWLIB_SRC)\stdio\*.c \
    $(NEWLIB_SRC)\string\*.c \
    $(NEWLIB_SRC)\stdlib\*.c

# Source files - updated to include terminal
C_SRC = $(wildcard kernel.c libc/*.c drivers/*.c fs/*.c)
ASM_SRC = # No boot.asm needed anymore

# Object files
C_OBJ = $(patsubst %.c, %.o, $(C_SRC))
ASM_OBJ = $(patsubst %.asm, %.o, $(ASM_SRC))

# Link Newlib objects
OBJS += $(NEWLIB_OBJS)

# Output files
KERNEL_ELF = kernel.elf
ISO_IMAGE = myos.iso

# GRUB files
GRUB_DIR = grub
GRUB_CFG = $(GRUB_DIR)/grub.cfg

# Default target
all: $(ISO_IMAGE)

# Build the kernel ELF
$(KERNEL_ELF): $(C_OBJ) $(ASM_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Create the ISO image
$(ISO_IMAGE): $(KERNEL_ELF) $(GRUB_CFG)
	mkdir -p iso_root/boot/grub
	cp $(KERNEL_ELF) iso_root/boot/kernel.elf
	cp $(GRUB_CFG) iso_root/boot/grub/
	grub-mkrescue -o $(ISO_IMAGE) iso_root

run-bios:
	qemu-system-i386 -cdrom myos.iso 

# Clean up
clean:
	rm -f $(C_OBJ) $(ASM_OBJ) $(KERNEL_ELF) $(ISO_IMAGE)
	rm -rf iso_root
