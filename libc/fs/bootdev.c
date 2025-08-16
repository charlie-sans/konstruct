#include "bootdev.h"
#include "iso9660.h"
#include "fat.h"
#include "../libc/libc.h"
#include "../drivers/vga.h"
#include <kernel.h>
#include "../../globals.h"  // Add the include for globals.h

// Globals to track boot device state
static boot_device_type_t boot_device_type = BOOT_DEV_UNKNOWN;
static int boot_device_initialized = 0;
static int boot_device_mounted = 0;
static char mount_path[256] = {0};
static boot_device_t boot_device;

// Multiboot information (used by bootdev.c)
uint32_t mboot_drive_number = 0x80;  // Default to first hard disk
uint32_t mboot_drive_part_start = 0;
uint32_t mboot_drive_part_length = 0;

// Create a Disk Address Packet (DAP)
typedef struct {
    uint8_t  packet_size; // Size of this packet (0x10)
    uint8_t  reserved;    // Always 0
    uint16_t sector_count; // Number of sectors to transfer (1)
    uint16_t buffer_offset; // Offset of buffer
    uint16_t buffer_segment;// Segment of buffer
    uint64_t lba_start;    // Starting LBA
} dap_t;
// Forward declarations for filesystem drivers
int iso9660_init(boot_device_t* dev);
int fat_init(boot_device_t* dev);

// Initialize the boot device
int bootdev_init(void) {
    printf("bootdev_init: Detecting boot device...\n");
    
    // For now, always assume it's a CDROM (for testing)
    boot_device_type = BOOT_DEV_CDROM;
    boot_device_initialized = 1;
    
    // Initialize the appropriate filesystem
    if (boot_device_type == BOOT_DEV_CDROM) {
        printf("bootdev_init: Initializing ISO9660 filesystem\n");
        int result = iso9660_init(&boot_device);
        if (result != BOOTDEV_SUCCESS) {
            printf("bootdev_init: ISO9660 initialization failed\n");
            return result;
        }
    } else if (boot_device_type == BOOT_DEV_FLOPPY) {
        printf("bootdev_init: Initializing FAT filesystem\n");
        int result = fat_init(&boot_device);
        if (result != BOOTDEV_SUCCESS) {
            printf("bootdev_init: FAT initialization failed\n");
            return result;
        }
    } else {
        printf("bootdev_init: Unknown boot device type\n");
        return BOOTDEV_ERROR_UNKNOWN;
    }
    
    printf("bootdev_init: Boot device initialized successfully\n");
    return BOOTDEV_SUCCESS;
}

// Mount the boot device at the specified path
int bootdev_mount(const char* path) {
    if (!boot_device_initialized) {
        printf("bootdev_mount: Boot device not initialized\n");
        return BOOTDEV_ERROR_NOT_INITIALIZED;
    }
    
    printf("bootdev_mount: Mounting boot device at %s\n", path);
    
    // Store the mount path
    strncpy(mount_path, path, sizeof(mount_path) - 1);
    mount_path[sizeof(mount_path) - 1] = '\0';
    
    boot_device_mounted = 1;
    printf("bootdev_mount: Boot device mounted successfully\n");
    
    return BOOTDEV_SUCCESS;
}

// Unmount the boot device
int bootdev_unmount(void) {
    if (!boot_device_mounted) {
        printf("bootdev_unmount: Boot device not mounted\n");
        return BOOTDEV_ERROR_NOT_MOUNTED;
    }
    
    printf("bootdev_unmount: Unmounting boot device\n");
    
    boot_device_mounted = 0;
    printf("bootdev_unmount: Boot device unmounted successfully\n");
    
    return BOOTDEV_SUCCESS;
}

// Get the boot device type
boot_device_type_t bootdev_get_type(void) {
    return boot_device_type;
}

// Get the boot device type name
const char* bootdev_get_type_name(void) {
    switch (boot_device_type) {
        case BOOT_DEV_FLOPPY:
            return "Floppy Disk";
        case BOOT_DEV_CDROM:
            return "CD-ROM";
        case BOOT_DEV_HDD:
            return "Hard Disk";
        case BOOT_DEV_USB:
            return "USB Drive";
        default:
            return "Unknown";
    }
}

// Check if boot device is mounted
int is_boot_device_mounted(void) {
    return boot_device_mounted;
}

// Set boot device mounted status (for global access)
void set_boot_device_mounted(int status) {
    boot_device_mounted = status;
}
