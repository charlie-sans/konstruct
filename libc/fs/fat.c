#include "fat.h"
#include "../libc/libc.h"

// Simple stub implementation for FAT filesystem
int fat_init(boot_device_t* dev) {
    printf("FAT filesystem not yet implemented\n");
    return BOOTDEV_ERROR_UNSUPPORTED;
}
