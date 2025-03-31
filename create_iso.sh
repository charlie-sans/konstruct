#!/bin/bash
# Script to create a bootable ISO with our custom OS

# Create temporary directory structure
mkdir -p iso/boot
cp myos.img iso/boot/

# Create the ISO image
echo "Creating bootable ISO image..."
mkisofs -o myos.iso -b boot/myos.img -c boot/boot.cat \
    -no-emul-boot -boot-load-size 4 -boot-info-table iso/

echo "Done! Your bootable ISO image (myos.iso) is ready."
echo "Burn this ISO to a CD/DVD using your preferred burning software."
echo "1. Insert the CD/DVD into your pc"
echo "2. Power on the computer and press F12 repeatedly to access the boot menu"
echo "3. Select the CD/DVD drive from the boot menu"

# Clean up
rm -rf iso/
