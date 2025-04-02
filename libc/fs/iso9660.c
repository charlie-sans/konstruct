#include "iso9660.h"
#include "bootdev.h"
#include "../libc/libc.h"

// Buffer for reading sectors
static uint8_t sector_buffer[ISO9660_SECTOR_SIZE];

// ISO9660 metadata
static iso9660_pvd_t* volume_descriptor = NULL;
static uint32_t root_directory_sector = 0;
static uint32_t root_directory_size = 0;

// Helper: Read a sector from the boot device into the buffer
static int read_sector(uint32_t sector_number) {
    return bootdev_read_sector(sector_number, sector_buffer);
}

// Helper: Parse a directory entry
static iso9660_dir_entry_t* get_next_dir_entry(iso9660_dir_entry_t* current) {
    if (current->length == 0) {
        return NULL; // End of entries in this sector
    }
    
    // Move to the next entry
    return (iso9660_dir_entry_t*)((char*)current + current->length);
}

// Initialize the ISO9660 filesystem driver
int iso9660_init(boot_device_t* dev) {
    // Allocate memory for the volume descriptor
    volume_descriptor = (iso9660_pvd_t*)malloc(sizeof(iso9660_pvd_t));
    if (!volume_descriptor) {
        printf("Failed to allocate memory for ISO9660 volume descriptor\n");
        return BOOTDEV_ERROR_INVALID;
    }
    
    // Read the volume descriptors, starting from sector 16
    uint32_t sector = 16;
    int found_pvd = 0;
    
    while (1) {
        if (read_sector(sector) != BOOTDEV_SUCCESS) {
            printf("Failed to read ISO9660 sector %d\n", sector);
            free(volume_descriptor);
            return BOOTDEV_ERROR_READ;
        }
        
        // Check if this is a valid volume descriptor
        if (memcmp(sector_buffer + 1, ISO9660_SIGNATURE, ISO9660_SIGNATURE_LEN) != 0) {
            printf("Invalid ISO9660 signature\n");
            free(volume_descriptor);
            return BOOTDEV_ERROR_INVALID;
        }
        
        // Check descriptor type
        uint8_t desc_type = sector_buffer[0];
        if (desc_type == ISO9660_PRIMARY_VOL_DESC) {
            // Found the primary volume descriptor
            memcpy(volume_descriptor, sector_buffer, sizeof(iso9660_pvd_t));
            found_pvd = 1;
            break;
        } else if (desc_type == ISO9660_END_VOL_DESC) {
            // End of volume descriptors
            break;
        }
        
        // Move to the next sector
        sector++;
    }
    
    if (!found_pvd) {
        printf("Primary volume descriptor not found\n");
        free(volume_descriptor);
        return BOOTDEV_ERROR_INVALID;
    }
    
    // Get root directory location
    root_directory_sector = volume_descriptor->root_dir_entry.extent_location_lsb;
    root_directory_size = volume_descriptor->root_dir_entry.data_length_lsb;
    
    // Store filesystem-specific data in the boot device
    dev->driver_data = volume_descriptor;
    
    printf("ISO9660 filesystem detected\n");
    printf("Volume: %.*s\n", 32, volume_descriptor->volume_id);
    
    return BOOTDEV_SUCCESS;
}

// Find a file or directory in the ISO9660 filesystem
static int find_entry(const char* path, iso9660_dir_entry_t** result) {
    // Start at the root directory
    uint32_t current_sector = root_directory_sector;
    uint32_t current_size = root_directory_size;
    uint32_t bytes_read = 0;
    
    // Copy path so we can modify it
    char* path_copy = strdup(path);
    char* token = strtok(path_copy, "/");
    
    // If path is empty or just "/", return the root directory
    if (!token) {
        free(path_copy);
        if (read_sector(current_sector) != BOOTDEV_SUCCESS) {
            return BOOTDEV_ERROR_READ;
        }
        *result = (iso9660_dir_entry_t*)sector_buffer;
        return BOOTDEV_SUCCESS;
    }
    
    while (token) {
        // Search for token in current directory
        int found = 0;
        bytes_read = 0;
        
        while (bytes_read < current_size) {
            // Read the next sector if needed
            if (bytes_read % ISO9660_SECTOR_SIZE == 0) {
                if (read_sector(current_sector + bytes_read / ISO9660_SECTOR_SIZE) != BOOTDEV_SUCCESS) {
                    free(path_copy);
                    return BOOTDEV_ERROR_READ;
                }
            }
            
            // Get the current directory entry
            iso9660_dir_entry_t* entry = (iso9660_dir_entry_t*)(sector_buffer + bytes_read % ISO9660_SECTOR_SIZE);
            
            // Check if we've reached the end of this sector's entries
            if (entry->length == 0) {
                // Move to the next sector
                bytes_read = (bytes_read / ISO9660_SECTOR_SIZE + 1) * ISO9660_SECTOR_SIZE;
                continue;
            }
            
            // Check if this entry matches the current path component
            // Skip "." and ".." entries
            if (entry->filename_length > 0 && entry->filename[0] != 0 && entry->filename[0] != 1) {
                // ISO9660 filenames typically end with ";1", so we need to strip that
                char filename[256];
                int filename_len = entry->filename_length;
                if (filename_len > 2 && entry->filename[filename_len - 2] == ';') {
                    filename_len -= 2;  // Remove ";1"
                }
                memcpy(filename, entry->filename, filename_len);
                filename[filename_len] = '\0';
                
                // Convert to uppercase for case-insensitive comparison
                for (int i = 0; i < filename_len; i++) {
                    if (filename[i] >= 'a' && filename[i] <= 'z') {
                        filename[i] = filename[i] - 'a' + 'A';
                    }
                }
                
                // Convert token to uppercase too
                char uptoken[256];
                strcpy(uptoken, token);
                for (int i = 0; uptoken[i]; i++) {
                    if (uptoken[i] >= 'a' && uptoken[i] <= 'z') {
                        uptoken[i] = uptoken[i] - 'a' + 'A';
                    }
                }
                
                // Compare
                if (strcmp(filename, uptoken) == 0) {
                    // Found the entry
                    found = 1;
                    
                    // Move to the next path component
                    token = strtok(NULL, "/");
                    
                    if (token) {
                        // If there are more path components, this must be a directory
                        if (!(entry->file_flags & 0x02)) {  // Not a directory
                            free(path_copy);
                            return BOOTDEV_ERROR_INVALID;
                        }
                        
                        // Move to this directory
                        current_sector = entry->extent_location_lsb;
                        current_size = entry->data_length_lsb;
                        bytes_read = 0;
                    } else {
                        // This is the last path component, return the entry
                        *result = malloc(entry->length);
                        if (!*result) {
                            free(path_copy);
                            return BOOTDEV_ERROR_INVALID;
                        }
                        memcpy(*result, entry, entry->length);
                        free(path_copy);
                        return BOOTDEV_SUCCESS;
                    }
                    
                    break;
                }
            }
            
            // Move to the next entry
            bytes_read += entry->length;
        }
        
        if (!found) {
            free(path_copy);
            return BOOTDEV_ERROR_NOT_FOUND;
        }
    }
    
    free(path_copy);
    return BOOTDEV_ERROR_NOT_FOUND;  // Should not reach here
}

// Read a file from the ISO9660 filesystem
int iso9660_read_file(const char* path, void* buffer, size_t size) {
    iso9660_dir_entry_t* entry = NULL;
    
    // Find the file
    int ret = find_entry(path, &entry);
    if (ret != BOOTDEV_SUCCESS) {
        return ret;
    }
    
    // Check if this is a file
    if (entry->file_flags & 0x02) {  // Directory
        free(entry);
        return BOOTDEV_ERROR_INVALID;
    }
    
    // Check if the buffer is large enough
    if (size < entry->data_length_lsb) {
        free(entry);
        return BOOTDEV_ERROR_INVALID;
    }
    
    // Read the file data
    uint32_t sector = entry->extent_location_lsb;
    uint32_t bytes_to_read = entry->data_length_lsb;
    uint32_t bytes_read = 0;
    
    while (bytes_read < bytes_to_read) {
        // Read the next sector
        if (read_sector(sector + bytes_read / ISO9660_SECTOR_SIZE) != BOOTDEV_SUCCESS) {
            free(entry);
            return BOOTDEV_ERROR_READ;
        }
        
        // Copy data to the buffer
        uint32_t bytes_in_this_sector = ISO9660_SECTOR_SIZE;
        if (bytes_read + bytes_in_this_sector > bytes_to_read) {
            bytes_in_this_sector = bytes_to_read - bytes_read;
        }
        
        memcpy((char*)buffer + bytes_read, sector_buffer, bytes_in_this_sector);
        bytes_read += bytes_in_this_sector;
    }
    
    free(entry);
    return bytes_read;
}

// List the contents of a directory
int iso9660_list_directory(const char* path, char* buffer, size_t size) {
    iso9660_dir_entry_t* entry = NULL;
    
    // Find the directory
    int ret = find_entry(path, &entry);
    if (ret != BOOTDEV_SUCCESS) {
        return ret;
    }
    
    // Check if this is a directory
    if (!(entry->file_flags & 0x02)) {  // Not a directory
        free(entry);
        return BOOTDEV_ERROR_INVALID;
    }
    
    // Read the directory contents
    uint32_t sector = entry->extent_location_lsb;
    uint32_t bytes_to_read = entry->data_length_lsb;
    uint32_t bytes_read = 0;
    
    // Start with an empty buffer
    buffer[0] = '\0';
    size_t buffer_used = 0;
    
    while (bytes_read < bytes_to_read) {
        // Read the next sector
        if (read_sector(sector + bytes_read / ISO9660_SECTOR_SIZE) != BOOTDEV_SUCCESS) {
            free(entry);
            return BOOTDEV_ERROR_READ;
        }
        
        // Process entries in this sector
        uint32_t offset = bytes_read % ISO9660_SECTOR_SIZE;
        while (offset < ISO9660_SECTOR_SIZE) {
            iso9660_dir_entry_t* dir_entry = (iso9660_dir_entry_t*)(sector_buffer + offset);
            
            // Check if we've reached the end of this sector's entries
            if (dir_entry->length == 0) {
                break;
            }
            
            // Skip "." and ".." entries
            if (dir_entry->filename_length > 0 && dir_entry->filename[0] != 0 && dir_entry->filename[0] != 1) {
                // ISO9660 filenames typically end with ";1", so we need to strip that
                char filename[256];
                int filename_len = dir_entry->filename_length;
                if (filename_len > 2 && dir_entry->filename[filename_len - 2] == ';') {
                    filename_len -= 2;  // Remove ";1"
                }
                memcpy(filename, dir_entry->filename, filename_len);
                filename[filename_len] = '\0';
                
                // Add to buffer if there's room
                int entry_len = strlen(filename) + 1;  // +1 for newline
                if (buffer_used + entry_len >= size) {
                    break;  // Buffer full
                }
                
                strcat(buffer, filename);
                strcat(buffer, "\n");
                buffer_used += entry_len;
            }
            
            // Move to the next entry
            offset += dir_entry->length;
        }
        
        bytes_read += ISO9660_SECTOR_SIZE;
    }
    
    free(entry);
    return buffer_used;
}
