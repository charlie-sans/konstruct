#include "iso9660.h"
#include "bootdev.h"
#include "../libc/libc.h"

// For simplicity, we'll create a mock implementation that simulates
// a basic ISO9660 filesystem with some predefined files

typedef struct {
    char name[32];
    char path[256];
    size_t size;
    char* data;
    int is_directory;
} iso9660_file_t;

#define MAX_ISO_FILES 32
static iso9660_file_t iso_files[MAX_ISO_FILES];
static int num_iso_files = 0;

// Initialize ISO9660 filesystem
int iso9660_init(boot_device_t* dev) {
    printf("Initializing ISO9660 filesystem...\n");
    
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
    iso_files[num_iso_files].data = strdup("Hello from CD-ROM filesystem!\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "readme.txt");
    strcpy(iso_files[num_iso_files].path, "/readme.txt");
    iso_files[num_iso_files].data = strdup("This is a simulated ISO9660 filesystem for development purposes.\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "version.txt");
    strcpy(iso_files[num_iso_files].path, "/etc/version.txt");
    iso_files[num_iso_files].data = strdup("konstruct OS v0.1\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "hello");
    strcpy(iso_files[num_iso_files].path, "/bin/hello");
    iso_files[num_iso_files].data = strdup("#!/bin/sh\necho Hello World\n");
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    iso_files[num_iso_files].is_directory = 0;
    strcpy(iso_files[num_iso_files].name, "test.prog");
    strcpy(iso_files[num_iso_files].path, "/programs/test.prog");
    
    // Create a simple program that outputs text
    char prog_data[] = "Program loaded and executed successfully!\n";
    iso_files[num_iso_files].data = strdup(prog_data);
    iso_files[num_iso_files].size = strlen(iso_files[num_iso_files].data);
    num_iso_files++;
    
    printf("ISO9660 filesystem initialized with %d files\n", num_iso_files);
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
    
    printf("iso9660_read_file: Reading %s\n", normalized_path);
    
    // Find the file
    for (int i = 0; i < num_iso_files; i++) {
        if (!iso_files[i].is_directory && strcmp(iso_files[i].path, normalized_path) == 0) {
            // Found the file
            size_t copy_size = size < iso_files[i].size ? size : iso_files[i].size;
            memcpy(buffer, iso_files[i].data, copy_size);
            printf("iso9660_read_file: Read %d bytes from %s\n", (int)copy_size, normalized_path);
            return copy_size;
        }
    }
    
    printf("iso9660_read_file: File not found: %s\n", normalized_path);
    return -1; // File not found
}

// List a directory from the ISO9660 filesystem
int iso9660_list_directory(const char* path, char* buffer, size_t size) {
    char normalized_path[256];
    normalize_path(path, normalized_path, sizeof(normalized_path));
    
    printf("iso9660_list_directory: Listing %s\n", normalized_path);
    
    // Special case for root directory
    if (strcmp(normalized_path, "/") == 0 || strcmp(normalized_path, "") == 0) {
        size_t offset = 0;
        
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
            }
        }
        
        printf("iso9660_list_directory: Listed root directory\n");
        return 0; // Success
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
        printf("iso9660_list_directory: Directory not found: %s\n", normalized_path);
        return -1; // Directory not found
    }
    
    // List the directory contents
    size_t offset = 0;
    size_t path_len = strlen(normalized_path);
    
    for (int i = 0; i < num_iso_files; i++) {
        // Check if this file is in the target directory
        if (strncmp(iso_files[i].path, normalized_path, path_len) == 0 &&
            iso_files[i].path[path_len] == '/' &&
            strchr(iso_files[i].path + path_len + 1, '/') == NULL) {
            
            // Extract just the filename
            const char* name = iso_files[i].path + path_len + 1;
            
            if (iso_files[i].is_directory) {
                offset += snprintf(buffer + offset, size - offset, "%s/\n", name);
            } else {
                offset += snprintf(buffer + offset, size - offset, "%s\n", name);
            }
        }
    }
    
    printf("iso9660_list_directory: Listed directory %s\n", normalized_path);
    return 0; // Success
}
