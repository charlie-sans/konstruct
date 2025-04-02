# Makefile for Konsole kernel with libc support

# Compiler settings
CC = gcc
AS = nasm
LD = ld

# Cross-compiler detection for macOS (commented out for now)
UNAME_S := $(shell uname -s)
# macOS cross-compiler setup would go here if needed

# Directories
SRC_DIR = .
LIBC_DIR = libc
DRIVERS_DIR = drivers
FS_DIR = fs
ELF_DIR = elf
SYSCALLS_DIR = syscalls
BUILD_DIR = build
ISO_DIR = iso_root

# GRUB files
GRUB_DIR = grub
GRUB_CFG = $(GRUB_DIR)/grub.cfg
GRUB_THEME_DIR = $(GRUB_DIR)/Particle

# Output files
KERNEL_ELF = kernel.elf
ISO_IMAGE = myos.iso

# Compiler and linker flags
CFLAGS = -m32 -ffreestanding -fno-pie -nostdlib -nostdinc \
         -fno-builtin -fno-stack-protector -nostartfiles \
         -nodefaultlibs -g
CFLAGS += -I$(SRC_DIR) -I$(LIBC_DIR)
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -Tlinker.ld

# Library
LIBC_LIB = $(LIBC_DIR)/libc.a

# Source files
C_SRC = $(wildcard $(SRC_DIR)/kernel.c) \
        $(wildcard $(DRIVERS_DIR)/*.c) \
        $(wildcard $(FS_DIR)/*.c) \
        $(wildcard $(ELF_DIR)/*.c) \
        $(wildcard $(SYSCALLS_DIR)/*.c) \
        $(wildcard $(SYSCALLS_DIR)/sys/*.c)

ASM_SRC = $(wildcard $(SRC_DIR)/*.asm)

# Object files
C_OBJ = $(patsubst %.c, %.o, $(C_SRC))
ASM_OBJ = $(patsubst %.asm, %.o, $(ASM_SRC))
OBJS += globals.o

# Ensure directories exist
$(shell mkdir -p $(DRIVERS_DIR) $(FS_DIR) $(ELF_DIR) $(SYSCALLS_DIR) $(SYSCALLS_DIR)/sys)

# Default target
all: $(ISO_IMAGE)

# Build the libc library
libc:
	$(MAKE) -C $(LIBC_DIR)

# Build the kernel
$(KERNEL_ELF): libc $(C_OBJ) $(ASM_OBJ)
	$(LD) $(LDFLAGS) -o $@ $(C_OBJ) $(ASM_OBJ) $(LIBC_LIB)

# Pattern rules for compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Create the ISO image
$(ISO_IMAGE): $(KERNEL_ELF) $(GRUB_CFG)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	cp $(GRUB_CFG) $(ISO_DIR)/boot/grub/
	mkdir -p $(ISO_DIR)/boot/grub/Particle
	cp -r $(GRUB_THEME_DIR)/ $(ISO_DIR)/boot/grub/Particle
	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR)

# Run in QEMU
run: $(ISO_IMAGE)
	qemu-system-i386 -cdrom $(ISO_IMAGE)

# Clean up
clean:
	rm -f $(C_OBJ) $(ASM_OBJ) $(KERNEL_ELF) $(ISO_IMAGE)
	rm -rf $(ISO_DIR)
	$(MAKE) -C $(LIBC_DIR) clean

# Debug target - print all variables
debug:
	@echo "C_SRC = $(C_SRC)"
	@echo "ASM_SRC = $(ASM_SRC)"
	@echo "C_OBJ = $(C_OBJ)"
	@echo "ASM_OBJ = $(ASM_OBJ)"
	@echo "KERNEL_ELF = $(KERNEL_ELF)"

run: $(ISO_IMAGE)
	qemu-system-i386 -cdrom $(ISO_IMAGE) 

.PHONY: all libc clean run debug