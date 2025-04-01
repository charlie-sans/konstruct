#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../libc/stdint.h"
#include "../libc/stddef.h"

// Maximum filename length
#define FS_MAX_FILENAME 32

// Maximum number of files in root directory
#define FS_MAX_FILES 64

// Maximum file size (256KB for simplicity)
#define FS_MAX_FILE_SIZE (256 * 1024)

// File types
#define FS_TYPE_FILE 1
#define FS_TYPE_DIRECTORY 2

// File flags
#define FS_FLAG_READABLE 0x01
#define FS_FLAG_WRITABLE 0x02
#define FS_FLAG_EXECUTABLE 0x04

// File descriptor structure
typedef struct {
    char filename[FS_MAX_FILENAME];
    uint32_t size;
    uint32_t type;
    uint32_t flags;
    uint32_t created;
    uint32_t modified;
    uint32_t data_offset; // Offset in data block or virtual disk
} fs_file_t;

// Filesystem status
typedef struct {
    uint32_t total_space;
    uint32_t used_space;
    uint32_t free_space;
    uint32_t file_count;
    uint32_t max_files;
} fs_status_t;

// Initialize the filesystem
int fs_init(void);

// Clean up filesystem resources
void fs_cleanup(void);

// Create a new file
int fs_create(const char* filename, uint32_t type, uint32_t flags);

// Open a file and return its descriptor index
int fs_open(const char* filename);

// Close a file
int fs_close(int fd);

// Read from a file
int fs_read(int fd, void* buffer, size_t size, size_t offset);

// Write to a file
int fs_write(int fd, const void* buffer, size_t size, size_t offset);

// Delete a file
int fs_delete(const char* filename);

// Rename a file
int fs_rename(const char* oldname, const char* newname);

// Get file information
int fs_stat(const char* filename, fs_file_t* info);

// List all files
int fs_list(fs_file_t* files, int max_count);

// Get filesystem status
int fs_get_status(fs_status_t* status);

// Mount a virtual disk (for persistence)
int fs_mount_disk(const char* diskname);

// Unmount the current virtual disk
int fs_unmount_disk(void);

// Sync in-memory filesystem to disk
int fs_sync(void);

#endif // FILESYSTEM_H
