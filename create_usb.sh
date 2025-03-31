#!/bin/bash
# Script to create a bootable USB drive with our custom OS

# Check if user provided USB device name
if [ $# -ne 1 ]; then
    echo "Usage: $0 <usb-device>"
    echo "Example: $0 /dev/sdb (Be VERY careful to specify the correct device!)"
    exit 1
fi

USB_DEVICE=$1

# Confirm with the user
echo "WARNING: This will ERASE ALL DATA on $USB_DEVICE"
echo "Please confirm this is the correct USB drive and not your system disk!"
read -p "Type 'yes' to continue: " confirm

if [ "$confirm" != "yes" ]; then
    echo "Operation cancelled"
    exit 1
fi

# Copy the OS image to the USB drive
echo "Writing OS image to $USB_DEVICE..."
dd if=myos.img of=$USB_DEVICE bs=512 conv=notrunc
sync

echo "Done! Your bootable USB drive is ready."
echo "1. Insert the USB drive into your Dell Optiplex 9010"
echo "2. Power on the computer and press F12 repeatedly to access the boot menu"
echo "3. Select the USB drive from the boot menu"
