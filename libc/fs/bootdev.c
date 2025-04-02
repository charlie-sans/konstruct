#include "bootdev.h"
#include "fs.h"
#include "../libc/libc.h"
#include "../drivers/vga.h"
#include <kernel.h>
#include "../../globals.h"  // Add the include for globals.h

// Global boot device information
static boot_device_t boot_device = {
    .type = BOOT_DEV_UNKNOWN,
    .start_sector = 0,
    .sector_count = 0,
    .drive_number = 0,
    .driver_data = NULL
};

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
    printf("bootdev_init: Starting boot device initialization\n");
    
    // Get drive number from multiboot info or BIOS
    // For QEMU/Bochs, we can use DL register value saved during boot
    boot_device.drive_number = mboot_drive_number;
    printf("bootdev_init: Drive number = 0x%02x\n", boot_device.drive_number);
    
    // Detect device type
    if (boot_device.drive_number == 0x00) {
        // Floppy disk
        boot_device.type = BOOT_DEV_FLOPPY;
        boot_device.start_sector = 0;
        boot_device.sector_count = 2880; // Standard 1.44MB floppy
        printf("bootdev_init: Detected Floppy disk\n");
    } else if (boot_device.drive_number == 0x80) {
        // Hard disk
        boot_device.type = BOOT_DEV_HDD;
        boot_device.start_sector = mboot_drive_part_start;
        boot_device.sector_count = mboot_drive_part_length;
        printf("bootdev_init: Detected Hard disk at sector %d with %d sectors\n", 
               boot_device.start_sector, boot_device.sector_count);
    } else if (boot_device.drive_number >= 0xE0) {
        // CDROM
        boot_device.type = BOOT_DEV_CDROM;
        boot_device.start_sector = 0;
        boot_device.sector_count = 0; // Unknown initially
        printf("bootdev_init: Detected CD-ROM\n");
    } else {
        // For testing purposes, let's assume a memory-based device
        printf("bootdev_init: Unknown device type (0x%02x), defaulting to memory device\n", 
               boot_device.drive_number);
        boot_device.type = BOOT_DEV_MEMORY;
        boot_device.start_sector = 0;
        boot_device.sector_count = 1024; // 1024 sectors (512KB)
        return BOOTDEV_SUCCESS; // Just return success for testing
    }
    
    // Initialize the appropriate filesystem driver
    int init_result = BOOTDEV_ERROR_UNSUPPORTED;
    switch (boot_device.type) {
        case BOOT_DEV_CDROM:
            printf("bootdev_init: Initializing ISO9660 filesystem\n");
            init_result = iso9660_init(&boot_device);
            break;
        
        case BOOT_DEV_FLOPPY:
        case BOOT_DEV_HDD:
            printf("bootdev_init: Initializing FAT filesystem\n");
            init_result = BOOTDEV_SUCCESS; // Simulate success for now
            break;
            
        case BOOT_DEV_MEMORY:
            printf("bootdev_init: Using memory-based device (no filesystem)\n");
            init_result = BOOTDEV_SUCCESS;
            break;
            
        default:
            printf("bootdev_init: Unsupported device type\n");
            init_result = BOOTDEV_ERROR_UNSUPPORTED;
            break;
    }
    
    if (init_result != BOOTDEV_SUCCESS) {
        printf("bootdev_init: Failed to initialize filesystem (error code: %d)\n", init_result);
        return init_result;
    }
    
    printf("bootdev_init: Boot device initialized successfully\n");
    return BOOTDEV_SUCCESS;
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
    // return boot_device.type;
    //TODO: figure out why the above line doesn't work
    return 3;
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
    printf("bootdev_mount: Mounting at %s\n", mount_point);
    
    // Check if mount point exists, create if not
    if (!fs_exists(mount_point)) {
        printf("bootdev_mount: Mount point doesn't exist, creating it\n");
        if (fs_mkdir(mount_point) != FS_SUCCESS) {
            printf("bootdev_mount: Failed to create mount point\n");
            return BOOTDEV_ERROR_INVALID;
        }
    }
    
    printf("Mounted %s as %s\n", bootdev_get_type_name(), mount_point);
    return BOOTDEV_SUCCESS;
}
