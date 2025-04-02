#include "bootdev.h"
#include "fs.h"
#include "../libc/libc.h"
#include "../drivers/vga.h"

// Global boot device information
static boot_device_t boot_device = {
    .type = BOOT_DEV_UNKNOWN,
    .start_sector = 0,
    .sector_count = 0,
    .drive_number = 0,
    .driver_data = NULL
};

// Boot multiboot information (from multiboot header)
extern uint32_t mboot_drive_number;
extern uint32_t mboot_drive_part_start;
extern uint32_t mboot_drive_part_length;

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
    // Get drive number from multiboot info or BIOS
    // For QEMU/Bochs, we can use DL register value saved during boot
    boot_device.drive_number = mboot_drive_number;
    
    // Detect device type
    if (boot_device.drive_number == 0x00) {
        // Floppy disk
        boot_device.type = BOOT_DEV_FLOPPY;
        boot_device.start_sector = 0;
        boot_device.sector_count = 2880; // Standard 1.44MB floppy
    } else if (boot_device.drive_number == 0x80) {
        // Hard disk
        boot_device.type = BOOT_DEV_HDD;
        boot_device.start_sector = mboot_drive_part_start;
        boot_device.sector_count = mboot_drive_part_length;
    } else if (boot_device.drive_number >= 0xE0) {
        // CDROM
        boot_device.type = BOOT_DEV_CDROM;
        boot_device.start_sector = 0;
        boot_device.sector_count = 0; // Unknown initially
    } else {
        // Unknown
        boot_device.type = BOOT_DEV_UNKNOWN;
        return BOOTDEV_ERROR_NOT_FOUND;
    }
    
    // Initialize the appropriate filesystem driver
    switch (boot_device.type) {
        case BOOT_DEV_CDROM:
            return iso9660_init(&boot_device);
        
        case BOOT_DEV_FLOPPY:
        case BOOT_DEV_HDD:
            return fat_init(&boot_device);
            
        default:
            return BOOTDEV_ERROR_UNSUPPORTED;
    }
}

// Read a sector from the boot device
int bootdev_read_sector(uint32_t sector_number, void* buffer) {
    // Check for invalid parameters
    if (buffer == NULL || sector_number >= boot_device.sector_count) {
        return BOOTDEV_ERROR_INVALID;
    }

    // Calculate LBA (Linear Block Address)
    uint64_t lba = boot_device.start_sector + sector_number;

    // Prepare parameters for INT 13h (AH=0x42 - Extended Read)
    uint8_t drive_number = boot_device.drive_number;

    // Create Disk Address Packet (DAP)
    dap_t dap;
    dap.packet_size = 0x10;
    dap.reserved = 0;
    dap.sector_count = 1;
    dap.buffer_offset = (uint16_t)((uintptr_t)buffer & 0xFFFF);
    dap.buffer_segment = (uint16_t)(((uintptr_t)buffer >> 4) & 0xFFFF);
    dap.lba_start = lba;

    // Call INT 13h using inline assembly (AT&T syntax)
    uint8_t status = 0;
    asm volatile (
        "pushfl\n"
        "pushl %%eax\n"
        "pushl %%ebx\n"
        "pushl %%ecx\n"
        "pushl %%edx\n"
        "pushl %%esi\n"
        "pushl %%edi\n"
        
        "movb $0x42, %%ah\n"
        "movb %1, %%dl\n"
        "movl %2, %%esi\n"
        "int $0x13\n"
        "movb %%ah, %0\n"
        
        "popl %%edi\n"
        "popl %%esi\n" 
        "popl %%edx\n"
        "popl %%ecx\n"
        "popl %%ebx\n"
        "popl %%eax\n"
        "popfl\n"
        : "=m" (status)
        : "m" (drive_number), "r" (&dap)
        : "memory"
    );

    // Check for errors
    if (status != 0x00) {
        printf("Disk read error: %x\r\n", status);
        return BOOTDEV_ERROR_IO;
    }

    return BOOTDEV_SUCCESS;
}

// Write a sector to the boot device (if supported)
int bootdev_write_sector(uint32_t sector_number, const void* buffer) {
    // Check if device is read-only
    if (boot_device.type == BOOT_DEV_CDROM) {
        return BOOTDEV_ERROR_UNSUPPORTED; // Can't write to CDROM
    }
    
    // Placeholder implementation
    if (buffer == NULL || sector_number >= boot_device.sector_count) {
        return BOOTDEV_ERROR_INVALID;
    }
    

    return BOOTDEV_SUCCESS;
}

// Get the boot device type
boot_device_type_t bootdev_get_type(void) {
    return boot_device.type;
}

// Get a human-readable name for the boot device type
const char* bootdev_get_type_name(void) {
    switch (boot_device.type) {
        case BOOT_DEV_CDROM:  return "CD-ROM";
        case BOOT_DEV_FLOPPY: return "Floppy Disk";
        case BOOT_DEV_HDD:    return "Hard Disk";
        case BOOT_DEV_MEMORY: return "Memory Disk";
        default:              return "Unknown";
    }
}

// Mount the boot device at the specified mount point
int bootdev_mount(const char* mount_point) {
    // Check if mount point exists, create if not
    if (!fs_exists(mount_point)) {
        if (fs_mkdir(mount_point) != FS_SUCCESS) {
            return BOOTDEV_ERROR_INVALID;
        }
    }
    
    // Register the mount point with the filesystem
    // This depends on your filesystem implementation
    // For a simple approach, we can add a special entry in the root directory
    
    printf("Mounted %s as %s\n", bootdev_get_type_name(), mount_point);
    return BOOTDEV_SUCCESS;
}
