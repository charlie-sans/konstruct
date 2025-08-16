#include "iso9660.h"
#include "bootdev.h"
#include "../libc/libc.h"

// For simplicity, we'll create a mock implementation that simulates
// a basic ISO9660 filesystem with some predefined files

typedef struct {
    char name[32];
    char path[256];
    size_t size;
    char data[512];  // Use static buffer instead of dynamic allocation
    int is_directory;
} iso9660_file_t;

#define MAX_ISO_FILES 32
static iso9660_file_t iso_files[MAX_ISO_FILES];
static int num_iso_files = 0;

// Initialize ISO9660 filesystem
int iso9660_init(boot_device_t* dev) {
    // Simple initialization without verbose output
    // Clear existing files
    memset(iso_files, 0, sizeof(iso_files));
    num_iso_files = 0;
    
    // Create some example directories
    iso_files[num_iso_files].is_directory = 1;
    strcpy(iso_files[num_iso_files].name, "bin");
    strcpy(iso_files[num_iso_files].path, "/bin");
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 1;
    strcpy(iso_files[num_iso_files].name, "etc");
    strcpy(iso_files[num_iso_files].path, "/etc");
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 1;
    strcpy(iso_files[num_iso_files].name, "programs");
    strcpy(iso_files[num_iso_files].path, "/programs");
    num_iso_files++;
    
    // Create some example files
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "hello.txt");
    strcpy(iso_files[num_iso_files].path, "/hello.txt");
    strcpy(iso_files[num_iso_files].data, "Hello from CD-ROM filesystem!\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "readme.txt");
    strcpy(iso_files[num_iso_files].path, "/readme.txt");
    strcpy(iso_files[num_iso_files].data, "This is a simulated ISO9660 filesystem for development purposes.\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "version.txt");
    strcpy(iso_files[num_iso_files].path, "/etc/version.txt");
    strcpy(iso_files[num_iso_files].data, "konstruct OS v0.1\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "hello");
    strcpy(iso_files[num_iso_files].path, "/bin/hello");
    strcpy(iso_files[num_iso_files].data, "#!/bin/sh\necho Hello World\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "test.prog");
    strcpy(iso_files[num_iso_files].path, "/programs/test.prog");
    
    // Create a simple program that outputs text
    strcpy(iso_files[num_iso_files].data, "Program loaded and executed successfully!\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    // Initialization complete - reduced output to avoid early boot issues
    return BOOTDEV_SUCCESS;
}

// Normalize path (remove trailing slash if present)
static void normalize_path(const char* path, char* normalized, size_t size) {
    strncpy(normalized, path, size);
    normalized[size-1] = '\0';
    
    // Remove trailing slash if present
    size_t len = strlen(normalized);
    if (len > 1 && normalized[len-1] == '/') {
        normalized[len-1] = '\0';
    }
}

// Read a file from the ISO9660 filesystem
int iso9660_read_file(const char* path, void* buffer, size_t size) {
    char normalized_path[256];
    normalize_path(path, normalized_path, sizeof(normalized_path));
    
    // Find the file
    for (int i = 0; i < num_iso_files; i++) {
        if (!iso_files[i].is_directory && strcmp(iso_files[i].path, normalized_path) == 0) {
            // Found the file
            size_t copy_size = size < iso_files[i].size ? size : iso_files[i].size;
            memcpy(buffer, iso_files[i].data, copy_size);
            return copy_size;
        }
    }
    
    return -1; // File not found
}

// List a directory from the ISO9660 filesystem
int iso9660_list_directory(const char* path, char* buffer, size_t size) {
    char normalized_path[256];
    normalize_path(path, normalized_path, sizeof(normalized_path));
    
    // Special case for root directory
    if (strcmp(normalized_path, "/") == 0 || strcmp(normalized_path, "") == 0) {
        size_t offset = 0;
        int file_count = 0;
        
        // Build the directory listing
        for (int i = 0; i < num_iso_files; i++) {
            // Only include files/directories directly in the root
            char* slash = strchr(iso_files[i].path + 1, '/');
            if (slash == NULL) {
                if (iso_files[i].is_directory) {
                    offset += snprintf(buffer + offset, size - offset, "%s/\n", iso_files[i].name);
                } else {
                    offset += snprintf(buffer + offset, size - offset, "%s\n", iso_files[i].name);
                }
                file_count++;
            }
        }
        
        return file_count; // Return the number of files found
    }
    
    // Check if the directory exists
    int dir_exists = 0;
    for (int i = 0; i < num_iso_files; i++) {
        if (iso_files[i].is_directory && strcmp(iso_files[i].path, normalized_path) == 0) {
            dir_exists = 1;
            break;
        }
    }
    
    if (!dir_exists) {
        return -1; // Directory not found
    }
    
    // List the directory contents
    size_t offset = 0;
    size_t path_len = strlen(normalized_path);
    int file_count = 0;
    
    for (int i = 0; i < num_iso_files; i++) {
        // Check if this file is in the target directory
        if (strncmp(iso_files[i].path, normalized_path, path_len) == 0) {
            // Make sure we're looking at a direct child of the directory, not a deeper path
            const char* remaining_path = iso_files[i].path + path_len;
            
            // Skip files that aren't direct children
            if (path_len > 1 && remaining_path[0] != '/') continue;
            if (path_len > 1 && strchr(remaining_path + 1, '/') != NULL) continue;
            
            // Skip the directory itself
            if (strlen(remaining_path) <= 1) continue;
            
            // Extract just the filename (skip the leading '/')
            const char* name = remaining_path;
            if (name[0] == '/') name++;
            
            if (iso_files[i].is_directory) {
                offset += snprintf(buffer + offset, size - offset, "%s/\n", name);
            } else {
                offset += snprintf(buffer + offset, size - offset, "%s\n", name);
            }
            file_count++;
        }
    }
    
    return file_count; // Return the number of files found
}
