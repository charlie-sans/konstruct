#include "filesystem.h"
#include "../libc/libc.h"

// Virtual disk format:
// Bytes 0-3: Magic number "DISK"
// Bytes 4-7: Disk size in bytes
// Bytes 8-11: Number of files
// Bytes 12-15: Directory size in bytes
// Bytes 16+: Directory entries, followed by file data

// Disk magic number
#define DISK_MAGIC 0x4B534944 // "DISK" in little-endian

// Virtual disk header structure
typedef struct {
    uint32_t magic;       // Magic number "DISK"
    uint32_t disk_size;   // Disk size in bytes
    uint32_t file_count;  // Number of files
    uint32_t dir_size;    // Directory size in bytes
} disk_header_t;

// Virtual disk state
static struct {
    int mounted;          // 1 if disk is mounted
    char* disk_path;      // Path to disk image file
    uint8_t* disk_data;   // Disk data (memory-mapped)
    uint32_t disk_size;   // Disk size in bytes
} vdisk;

// Forward declarations of helper functions
static int vdisk_load(const char* path);
static int vdisk_save(void);
static int vdisk_create(const char* path, uint32_t size);

// Mount a virtual disk (load it into memory)
int fs_mount_disk(const char* diskpath) {
    // Check if a disk is already mounted
    if (vdisk.mounted) {
        printf("A disk is already mounted\n");
        return -1;
    }
    
    // Try to load the disk
    if (vdisk_load(diskpath) < 0) {
        // If loading fails, create a new disk
        printf("Disk not found, creating new disk at %s\n", diskpath);
        
        // Create a new 8MB disk
        if (vdisk_create(diskpath, 8 * 1024 * 1024) < 0) {
            printf("Failed to create disk\n");
            return -1;
        }
        
        // Load the newly created disk
        if (vdisk_load(diskpath) < 0) {
            printf("Failed to load newly created disk\n");
            return -1;
        }
    }
    
    printf("Mounted disk: %s\n", diskpath);
    
    // You would now initialize the ramdisk from the virtual disk data
    // This implementation is left as an exercise
    
    return 0;
}

// Unmount the virtual disk (save it back to the disk image file)
int fs_unmount_disk(void) {
    if (!vdisk.mounted) {
        printf("No disk is mounted\n");
        return -1;
    }
    
    // Save disk data back to the file
    if (vdisk_save() < 0) {
        printf("Failed to save disk\n");
        return -1;
    }
    
    // Free resources
    if (vdisk.disk_data) {
        free(vdisk.disk_data);
        vdisk.disk_data = NULL;
    }
    
    if (vdisk.disk_path) {
        free(vdisk.disk_path);
        vdisk.disk_path = NULL;
    }
    
    vdisk.mounted = 0;
    vdisk.disk_size = 0;
    
    printf("Disk unmounted\n");
    return 0;
}

// Sync in-memory filesystem to disk (save disk image)
int fs_sync(void) {
    if (!vdisk.mounted) {
        printf("No disk is mounted\n");
        return -1;
    }
    
    // Save disk data back to the file
    if (vdisk_save() < 0) {
        printf("Failed to sync disk\n");
        return -1;
    }
    
    printf("Disk synced\n");
    return 0;
}

// Helper function to load a disk image into memory
static int vdisk_load(const char* path) {
    // In a real OS, this would use file I/O to load the disk image
    // For now, we'll simulate it with a stub
    
    printf("Loading disk image: %s\n", path);
    
    // Store disk path
    vdisk.disk_path = strdup(path);
    if (!vdisk.disk_path) {
        printf("Failed to allocate memory for disk path\n");
        return -1;
    }
    
    // Set mounted flag
    vdisk.mounted = 1;
    
    // For the simulator, we'll create an empty disk in memory
    vdisk.disk_size = 8 * 1024 * 1024;  // 8MB
    vdisk.disk_data = (uint8_t*)malloc(vdisk.disk_size);
    
    if (!vdisk.disk_data) {
        printf("Failed to allocate memory for disk data\n");
        free(vdisk.disk_path);
        vdisk.disk_path = NULL;
        vdisk.mounted = 0;
        return -1;
    }
    
    // Initialize disk header
    disk_header_t* header = (disk_header_t*)vdisk.disk_data;
    header->magic = DISK_MAGIC;
    header->disk_size = vdisk.disk_size;
    header->file_count = 0;
    header->dir_size = 0;
    
    return 0;
}

// Helper function to save disk data back to the disk image file
static int vdisk_save(void) {
    // In a real OS, this would use file I/O to save the disk image
    // For now, we'll simulate it with a stub
    
    if (!vdisk.mounted || !vdisk.disk_data || !vdisk.disk_path) {
        return -1;
    }
    
    printf("Saving disk image: %s\n", vdisk.disk_path);
    
    // In a real implementation, you would write vdisk.disk_data to a file
    
    return 0;
}

// Helper function to create a new disk image
static int vdisk_create(const char* path, uint32_t size) {
    // In a real OS, this would use file I/O to create a new disk image
    // For now, we'll simulate it with a stub
    
    printf("Creating new disk image: %s (%d bytes)\n", path, size);
    
    // In a real implementation, you would create a new file of the given size
    
    return 0;
}
