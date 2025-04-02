#ifndef FS_H
#define FS_H

#include "../libc/libc.h"

// Define time_t since we don't have standard library
typedef unsigned long time_t;

// Filesystem constants
#define FS_MAX_FILENAME_LENGTH 32
#define FS_MAX_PATH_LENGTH 256
#define FS_MAX_FILES 64
#define FS_MAX_DIRS 16
#define FS_MAX_FILE_SIZE (64 * 1024)  // 64KB max file size

// Filesystem error codes
#define FS_SUCCESS 0
#define FS_ERR_NOT_FOUND -1
#define FS_ERR_EXISTS -2
#define FS_ERR_NO_SPACE -3
#define FS_ERR_INVALID -4
#define FS_ERR_NOT_DIRECTORY -5
#define FS_ERR_NOT_FILE -6
#define FS_ERR_PERMISSION -7

// File types
#define FS_TYPE_FILE 1
#define FS_TYPE_DIRECTORY 2

// File permissions
#define FS_PERM_READ 1
#define FS_PERM_WRITE 2
#define FS_PERM_EXECUTE 4

// File structure
typedef struct {
    char name[FS_MAX_FILENAME_LENGTH];
    uint8_t type;              // File or directory
    uint8_t permissions;       // Read/write/execute permissions
    size_t size;               // Size of file in bytes
    void* data;                // Pointer to file data
    time_t created;            // Creation timestamp
    time_t modified;           // Last modified timestamp
    struct fs_dir* parent;     // Parent directory
} fs_file_t;

// Directory structure
typedef struct fs_dir {
    char name[FS_MAX_FILENAME_LENGTH];
    fs_file_t* files[FS_MAX_FILES];
    int file_count;
    struct fs_dir* subdirs[FS_MAX_DIRS];
    int subdir_count;
    struct fs_dir* parent;
} fs_dir_t;

// Filesystem structure
typedef struct {
    fs_dir_t* root;
    fs_dir_t* current_dir;
} filesystem_t;

// Global filesystem
extern filesystem_t* fs;

// Filesystem initialization and cleanup
void fs_init(void);
void fs_cleanup(void);

// Directory operations
int fs_mkdir(const char* path);
int fs_rmdir(const char* path);
int fs_chdir(const char* path);
char* fs_getcwd(char* buf, size_t size);
int fs_listdir(const char* path, char* buffer, size_t bufsize);

// File operations
int fs_create(const char* path);
int fs_delete(const char* path);
int fs_read(const char* path, void* buffer, size_t size, size_t offset);
int fs_write(const char* path, const void* buffer, size_t size, size_t offset);
int fs_getsize(const char* path);

// Load and execute a program from the filesystem
int fs_load_program(const char* path, void* buffer, size_t* size);

// Utility functions
fs_dir_t* fs_find_dir(const char* path);
fs_file_t* fs_find_file(const char* path);
char* fs_dirname(const char* path, char* buffer, size_t bufsize);
char* fs_basename(const char* path, char* buffer, size_t bufsize);
void fs_normalize_path(const char* path, char* normalized, size_t size);
int fs_exists(const char* path);

// Built-in file system testing and examples
void fs_create_example_files(void);

#endif // FS_H
