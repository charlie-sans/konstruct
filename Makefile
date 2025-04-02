# Makefile for building the Konsole kernel with Newlib support
# though newlib doesnt even exist in this version of the kernel, it is still included for future use if anyting needs it 
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
CFLAGS =  -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs  -I. -g
CFLAGS += -I. -Id:\tempsting\newlib\include -Ilibc/
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

# Ensure syscalls/sys directory exists
$(shell mkdir -p syscalls/sys)

# Define object directory
OBJ_DIR = obj

# Source files - updated to include terminal, elf, and syscalls
C_SRC = $(wildcard kernel.c libc/*.c drivers/*.c fs/*.c elf/*.c syscalls/*.c syscalls/sys/*.c)
ASM_SRC = # No boot.asm needed anymore

# Object files with obj directory prefix
C_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(C_SRC))
ASM_OBJ = $(patsubst %.asm, $(OBJ_DIR)/%.o, $(ASM_SRC))

# Link Newlib objects
OBJS += $(NEWLIB_OBJS)

# Output files
KERNEL_ELF = kernel.elf
ISO_IMAGE = myos.iso

# GRUB files
GRUB_DIR = grub
GRUB_CFG = $(GRUB_DIR)/grub.cfg
GRUB_THEME_DIR = $(GRUB_DIR)/Particle

# Default target
all: $(ISO_IMAGE)

# Create necessary directories
$(shell mkdir -p $(OBJ_DIR)/libc $(OBJ_DIR)/drivers $(OBJ_DIR)/fs $(OBJ_DIR)/elf $(OBJ_DIR)/syscalls $(OBJ_DIR)/syscalls/sys)

# Build the kernel ELF
$(KERNEL_ELF): $(C_OBJ) $(ASM_OBJ)
	$(LD) $(LDFLAGS) -o $@ $(C_OBJ) $(ASM_OBJ)

# Pattern rule for C files
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for ASM files
$(OBJ_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Create the ISO image
$(ISO_IMAGE): $(KERNEL_ELF) $(GRUB_CFG)
	mkdir -p iso_root/boot/grub
	cp $(KERNEL_ELF) iso_root/boot/kernel.elf
	cp $(GRUB_CFG) iso_root/boot/grub/
	mkdir -p iso_root/boot/grub/Particle
	cp -r $(GRUB_THEME_DIR)/ iso_root/boot/grub/Particle
	grub-mkrescue -o $(ISO_IMAGE) iso_root

run-bios:
	qemu-system-i386 -cdrom myos.iso 

# Clean up (updated to remove obj directory)
clean:
	rm -f $(C_OBJ) $(ASM_OBJ) $(KERNEL_ELF) $(ISO_IMAGE)
	rm -rf iso_root $(OBJ_DIR)

psudo:
# print everyting
	echo $(C_SRC)
	echo $(ASM_SRC)
	echo $(C_OBJ)
	echo $(ASM_OBJ)
	echo $(NEWLIB_OBJS)
	echo $(OBJS)
	echo $(KERNEL_ELF)