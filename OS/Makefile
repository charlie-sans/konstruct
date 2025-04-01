# Makefile for building our OS with UEFI support and libc

# Compiler settings
CC = gcc
AS = nasm
LD = ld

# Directories
SRC_DIR = src
MLIBC_DIR = ../MLibc
MLIBC_SRC = $(MLIBC_DIR)/src
MLIBC_INCLUDE = $(MLIBC_DIR)/include

# Flags for legacy BIOS build
CFLAGS_BIOS = -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -I$(SRC_DIR) -I$(MLIBC_INCLUDE)
ASFLAGS_BIOS = -f bin
LDFLAGS_BIOS = -m elf_i386 -T linker.ld --oformat binary -static

# Flags for UEFI build
CFLAGS_UEFI = -fno-stack-protector -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPPER
LDFLAGS_UEFI = -shared -Bsymbolic -L/usr/lib -T uefi_linker.ld

# UEFI includes and libraries
UEFI_INCLUDES = -I/usr/include/efi -I/usr/include/efi/x86_64
UEFI_LIBS = -lefi -lgnuefi

# Source files
BOOT_SRC = $(SRC_DIR)/boot.asm
KERNEL_SRC = $(SRC_DIR)/kernel.c
LIBC_SRC = $(MLIBC_SRC)/string.c $(MLIBC_SRC)/memory.c $(MLIBC_SRC)/stdio.c
BOOTLOADER_SRC = $(SRC_DIR)/bootloader.c

# Output files
OS_IMAGE = myos.img
KERNEL_BIN = kernel.bin
BOOT_BIN = boot.bin
BOOTLOADER_EFI = bootloader.efi

# Object files
KERNEL_OBJ = kernel.o
LIBC_OBJS = $(LIBC_SRC:.c=.o)

# Default target
all: bios

# BIOS boot target
bios: $(OS_IMAGE)

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "Creating disk image..."
	dd if=/dev/zero of=$(OS_IMAGE) bs=1024 count=1440
	dd if=$(BOOT_BIN) of=$(OS_IMAGE) conv=notrunc
	dd if=$(KERNEL_BIN) of=$(OS_IMAGE) seek=1 conv=notrunc
	@echo "Disk image created."

$(BOOT_BIN): $(BOOT_SRC)
	@echo "Assembling bootloader..."
	$(AS) $(ASFLAGS_BIOS) $(BOOT_SRC) -o $(BOOT_BIN)

$(KERNEL_BIN): $(KERNEL_OBJ) $(LIBC_OBJS)
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS_BIOS) -o $(KERNEL_BIN) $(KERNEL_OBJ) $(LIBC_OBJS)

%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS_BIOS) -c $< -o $@

# UEFI boot target
uefi: $(KERNEL_BIN) $(BOOTLOADER_EFI)
	@echo "Creating UEFI image..."
	mkdir -p uefi_image/EFI/BOOT
	cp $(KERNEL_BIN) uefi_image/
	cp $(BOOTLOADER_EFI) uefi_image/EFI/BOOT/BOOTX64.EFI
	@echo "UEFI image created."

$(BOOTLOADER_EFI): $(BOOTLOADER_SRC)
	@echo "Building UEFI bootloader..."
	$(CC) $(CFLAGS_UEFI) $(UEFI_INCLUDES) -c $(BOOTLOADER_SRC) -o bootloader.o
	$(LD) $(LDFLAGS_UEFI) -o $(BOOTLOADER_EFI) bootloader.o $(UEFI_LIBS)

# Run targets
run-bios: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE),index=0,if=floppy

run-uefi: uefi
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive file=fat:rw:uefi_image,format=raw

clean:
	@echo "Cleaning..."
	rm -f $(SRC_DIR)/*.o $(MLIBC_SRC)/*.o *.o *.bin *.img *.efi
	rm -rf uefi_image

.PHONY: all bios uefi run-bios run-uefi clean
