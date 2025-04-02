#ifndef BOOTDEV_H
#define BOOTDEV_H

#include <stdint.h>

// Boot device types
typedef enum {
    BOOT_DEV_UNKNOWN = 0,
    BOOT_DEV_FLOPPY,
    BOOT_DEV_HDD,
    BOOT_DEV_CDROM,
    BOOT_DEV_MEMORY  // For testing/simulation
} boot_device_type_t;

// Boot device structure
typedef struct {
    boot_device_type_t type;
    uint32_t drive_number;
    uint32_t start_sector;
    uint32_t sector_count;
    void* driver_data;     // Driver-specific data
} boot_device_t;

// Error codes
#define BOOTDEV_SUCCESS          0
#define BOOTDEV_ERROR_NOT_FOUND  1
#define BOOTDEV_ERROR_IO         2
#define BOOTDEV_ERROR_INVALID    3
#define BOOTDEV_ERROR_UNSUPPORTED 4
#define BOOTDEV_ERROR_READ      5

// External variable declarations
extern uint32_t mboot_drive_number;
extern uint32_t mboot_drive_part_start;
extern uint32_t mboot_drive_part_length;

// Function prototypes
int bootdev_init(void);
int bootdev_read_sector(uint32_t sector_number, void* buffer);
int bootdev_write_sector(uint32_t sector_number, const void* buffer);
boot_device_type_t bootdev_get_type(void);
const char* bootdev_get_type_name(void);
int bootdev_mount(const char* mount_point);

#endif // BOOTDEV_H
