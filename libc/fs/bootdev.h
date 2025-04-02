#ifndef BOOTDEV_H
#define BOOTDEV_H

#include "../libc/stdint.h"

// Boot device types
typedef enum {
    BOOT_DEV_UNKNOWN,
    BOOT_DEV_CDROM,    // ISO9660 format
    BOOT_DEV_FLOPPY,   // FAT12
    BOOT_DEV_HDD,      // FAT32/16 or other formats
    BOOT_DEV_MEMORY    // RAM disk
} boot_device_type_t;

// Boot device structure
typedef struct {
    boot_device_type_t type;   // Type of boot device
    uint32_t start_sector;     // Where the device starts
    uint32_t sector_count;     // Number of sectors
    uint8_t  drive_number;     // BIOS drive number
    void*    driver_data;      // Driver-specific data
} boot_device_t;

// Boot device functions
int bootdev_init(void);
int bootdev_read_sector(uint32_t sector_number, void* buffer);
int bootdev_write_sector(uint32_t sector_number, const void* buffer);
boot_device_type_t bootdev_get_type(void);
const char* bootdev_get_type_name(void);
int bootdev_mount(const char* mount_point);

// Error codes
#define BOOTDEV_SUCCESS           0
#define BOOTDEV_ERROR_NOT_FOUND  -1
#define BOOTDEV_ERROR_READ       -2
#define BOOTDEV_ERROR_WRITE      -3
#define BOOTDEV_ERROR_INVALID    -4
#define BOOTDEV_ERROR_UNSUPPORTED -5
#define BOOTDEV_ERROR_IO        -6
#define BOOTDEV_ERROR_MEMORY    -7

#endif // BOOTDEV_H
