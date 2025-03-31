#!/bin/bash
# Script to create a UEFI bootable image

# Create directory structure for UEFI image
mkdir -p uefi_image/EFI/BOOT

# Copy kernel and bootloader
cp kernel.bin uefi_image/
cp bootloader.efi uefi_image/EFI/BOOT/BOOTX64.EFI

echo "UEFI bootable image created in 'uefi_image' directory"
echo "Run with: make run-uefi"
