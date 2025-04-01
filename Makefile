# Makefile for building our OS with UEFI support and libc

# Compiler settings
CC = gcc
AS = nasm
LD = ld

# Flags for legacy BIOS build
CFLAGS_BIOS = -m32 -ffreestanding -fno-pie -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -I.
ASFLAGS_BIOS = -f elf32
LDFLAGS_BIOS = -m elf_i386 -Tlinker.ld --oformat binary -static

# Flags for UEFI build
CFLAGS_UEFI = -fno-stack-protector -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPPER
LDFLAGS_UEFI = -shared -Bsymbolic -L/usr/lib -T uefi_linker.ld

# UEFI includes and libraries
UEFI_INCLUDES = -I/usr/include/efi -I/usr/include/efi/x86_64
UEFI_LIBS = -lefi -lgnuefi

# Automatically find all source files
C_SRC = $(wildcard kernel.c libc/*.c drivers/*.c programs/*.c)
ASM_SRC = $(wildcard boot.asm)

# Object files
C_OBJ = $(patsubst %.c, %.o, $(C_SRC))
ASM_OBJ = $(patsubst %.asm, %.o, $(ASM_SRC))

# Separate bootloader and kernel object files
BOOT_BIN = boot.bin
KERNEL_OBJ = $(filter-out boot.asm, $(C_OBJ))  # Exclude boot.asm from kernel objects

# Output files
OS_IMAGE = myos.img
KERNEL_BIN = kernel.bin
BOOTLOADER_EFI = bootloader.efi

# Default target
all: bios

# BIOS boot target
bios: $(OS_IMAGE)

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	# Create a new blank disk image (1.44MB floppy)
	dd if=/dev/zero of=$(OS_IMAGE) bs=1024 count=1440
	# Write the bootloader to the first sector
	dd if=$(BOOT_BIN) of=$(OS_IMAGE) conv=notrunc
	# Write the kernel starting at the second sector
	dd if=$(KERNEL_BIN) of=$(OS_IMAGE) seek=1 conv=notrunc

$(BOOT_BIN): boot.asm
	nasm -f bin boot.asm -o $(BOOT_BIN)

$(KERNEL_BIN): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS_BIOS) -o $(KERNEL_BIN) $(KERNEL_OBJ)

%.o: %.c
	$(CC) $(CFLAGS_BIOS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS_BIOS) $< -o $@

# UEFI boot target
uefi: $(KERNEL_BIN) $(BOOTLOADER_EFI)
	./create_uefi_image.sh

$(BOOTLOADER_EFI): bootloader.c
	$(CC) $(CFLAGS_UEFI) $(UEFI_INCLUDES) -c bootloader.c -o bootloader.o
	$(LD) $(LDFLAGS_UEFI) -o $(BOOTLOADER_EFI) bootloader.o $(UEFI_LIBS)

# Run targets
run-bios: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE),index=0,if=floppy

run-uefi: uefi
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive file=fat:rw:uefi_image,format=raw

clean:
	rm -f *.o libc/*.o drivers/*.o *.bin *.img *.efi
	rm -rf uefi_image
