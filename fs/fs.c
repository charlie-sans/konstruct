#include "fs.h"

// Global filesystem instance
filesystem_t* fs = NULL;

// Helper function to get current time
static time_t get_time(void) {
    // In a real OS, this would get the actual system time
    // For now, just return a counter that increments each time
    static time_t time_counter = 0;
    return ++time_counter;
}

// Initialize the filesystem
void fs_init(void) {
    printf("Initializing filesystem...\n");
    
    // Allocate memory for filesystem
    fs = (filesystem_t*)malloc(sizeof(filesystem_t));
    if (!fs) {
        printf("Failed to allocate memory for filesystem\n");
        return;
    }
    
    // Create root directory
    fs->root = (fs_dir_t*)malloc(sizeof(fs_dir_t));
    if (!fs->root) {
        printf("Failed to allocate memory for root directory\n");
        free(fs);
        fs = NULL;
        return;
    }
    
    // Initialize root directory
    memset(fs->root, 0, sizeof(fs_dir_t));
    strcpy(fs->root->name, "/");
    fs->root->parent = NULL;  // Root has no parent
    
    // Set current directory to root
    fs->current_dir = fs->root;
    
    printf("Filesystem initialized successfully\n");
    
    // Create some example files and directories
    fs_create_example_files();
}

// Clean up the filesystem
void fs_cleanup(void) {
    // Recursive function to free directory and its contents
    void free_dir(fs_dir_t* dir) {
        if (!dir) return;
        
        // Free all files in the directory
        for (int i = 0; i < dir->file_count; i++) {
            if (dir->files[i]) {
                if (dir->files[i]->data) {
                    free(dir->files[i]->data);
                }
                free(dir->files[i]);
            }
        }
        
        // Recursively free all subdirectories
        for (int i = 0; i < dir->subdir_count; i++) {
            if (dir->subdirs[i]) {
                free_dir(dir->subdirs[i]);
            }
        }
        
        // Free the directory itself
        free(dir);
    }
    
    if (fs && fs->root) {
        free_dir(fs->root);
    }
    
    if (fs) {
        free(fs);
        fs = NULL;
    }
    
    printf("Filesystem cleaned up\n");
}

// Normalize a file path
void fs_normalize_path(const char* path, char* normalized, size_t size) {
    if (!path || !normalized || size == 0) return;
    
    // Initialize normalized path
    normalized[0] = '\0';
    
    // Handle empty path
    if (path[0] == '\0') {
        strncpy(normalized, "/", size);
        return;
    }
    
    // Copy the path
    strncpy(normalized, path, size);
    normalized[size - 1] = '\0'; // Ensure null termination
    
    // Replace backslashes with forward slashes
    for (char* p = normalized; *p; p++) {
        if (*p == '\\') *p = '/';
    }
    
    // TODO: Handle path normalization (e.g., ".." and ".")
    // For simplicity, we're not implementing this now
}

// Find a directory by path
fs_dir_t* fs_find_dir(const char* path) {
    if (!fs || !fs->root || !path) return NULL;
    
    char normalized_path[FS_MAX_PATH_LENGTH];
    fs_normalize_path(path, normalized_path, sizeof(normalized_path));
    
    // Handle root directory
    if (strcmp(normalized_path, "/") == 0) {
        return fs->root;
    }
    
    // Handle current directory
    if (strcmp(normalized_path, ".") == 0) {
        return fs->current_dir;
    }
    
    // Handle absolute vs relative paths
    fs_dir_t* current = normalized_path[0] == '/' ? fs->root : fs->current_dir;
    
    // Skip leading slash for absolute paths
    const char* p = normalized_path[0] == '/' ? normalized_path + 1 : normalized_path;
    
    // Tokenize the path and traverse directories
    char path_copy[FS_MAX_PATH_LENGTH];
    strncpy(path_copy, p, sizeof(path_copy));
    
    char* token = strtok(path_copy, "/");
    while (token && current) {
        if (strcmp(token, ".") == 0) {
            // Current directory, do nothing
        } else if (strcmp(token, "..") == 0) {
            // Parent directory
            if (current->parent) {
                current = current->parent;
            }
        } else {
            // Look for subdirectory
            int found = 0;
            for (int i = 0; i < current->subdir_count; i++) {
                if (strcmp(current->subdirs[i]->name, token) == 0) {
                    current = current->subdirs[i];
                    found = 1;
                    break;
                }
            }
            
            if (!found) {
                return NULL; // Directory not found
            }
        }
        
        token = strtok(NULL, "/");
    }
    
    return current;
}

// Find a file by path
fs_file_t* fs_find_file(const char* path) {
    if (!fs || !fs->root || !path) return NULL;
    
    char normalized_path[FS_MAX_PATH_LENGTH];
    fs_normalize_path(path, normalized_path, sizeof(normalized_path));
    
    // Extract directory path and filename
    char dir_path[FS_MAX_PATH_LENGTH];
    char filename[FS_MAX_FILENAME_LENGTH];
    
    // Find the last slash in the path
    char* last_slash = strrchr(normalized_path, '/');
    if (last_slash) {
        // Extract directory path
        int dir_len = last_slash - normalized_path;
        strncpy(dir_path, normalized_path, dir_len);
        dir_path[dir_len] = '\0';
        
        // If dir_path is empty, use root
        if (dir_path[0] == '\0') {
            strcpy(dir_path, "/");
        }
        
        // Extract filename
        strcpy(filename, last_slash + 1);
    } else {
        // No slash, assume file is in current directory
        strcpy(dir_path, ".");
        strcpy(filename, normalized_path);
    }
    
    // Find the directory
    fs_dir_t* dir = fs_find_dir(dir_path);
    if (!dir) return NULL;
    
    // Find the file in the directory
    for (int i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, filename) == 0) {
            return dir->files[i];
        }
    }
    
    return NULL;
}

// Create a directory
int fs_mkdir(const char* path) {
    if (!fs || !path) return FS_ERR_INVALID;
    
    char normalized_path[FS_MAX_PATH_LENGTH];
    fs_normalize_path(path, normalized_path, sizeof(normalized_path));
    
    // Extract parent directory path and new directory name
    char parent_path[FS_MAX_PATH_LENGTH];
    char dirname[FS_MAX_FILENAME_LENGTH];
    
    // Find the last slash in the path
    char* last_slash = strrchr(normalized_path, '/');
    if (last_slash) {
        // Extract parent directory path
        int parent_len = last_slash - normalized_path;
        strncpy(parent_path, normalized_path, parent_len);
        parent_path[parent_len] = '\0';
        
        // If parent_path is empty, use root
        if (parent_path[0] == '\0') {
            strcpy(parent_path, "/");
        }
        
        // Extract directory name
        strcpy(dirname, last_slash + 1);
    } else {
        // No slash, assume parent is current directory
        strcpy(parent_path, ".");
        strcpy(dirname, normalized_path);
    }
    
    // Find the parent directory
    fs_dir_t* parent = fs_find_dir(parent_path);
    if (!parent) return FS_ERR_NOT_FOUND;
    
    // Check if directory already exists
    for (int i = 0; i < parent->subdir_count; i++) {
        if (strcmp(parent->subdirs[i]->name, dirname) == 0) {
            return FS_ERR_EXISTS;
        }
    }
    
    // Check if name is already used by a file
    for (int i = 0; i < parent->file_count; i++) {
        if (strcmp(parent->files[i]->name, dirname) == 0) {
            return FS_ERR_EXISTS;
        }
    }
    
    // Check if we have room for another directory
    if (parent->subdir_count >= FS_MAX_DIRS) {
        return FS_ERR_NO_SPACE;
    }
    
    // Create the new directory
    fs_dir_t* new_dir = (fs_dir_t*)malloc(sizeof(fs_dir_t));
    if (!new_dir) return FS_ERR_NO_SPACE;
    
    // Initialize the new directory
    memset(new_dir, 0, sizeof(fs_dir_t));
    strncpy(new_dir->name, dirname, FS_MAX_FILENAME_LENGTH - 1);
    new_dir->parent = parent;
    
    // Add the new directory to the parent
    parent->subdirs[parent->subdir_count++] = new_dir;
    
    return FS_SUCCESS;
}

// Remove a directory
int fs_rmdir(const char* path) {
    if (!fs || !path) return FS_ERR_INVALID;
    
    // Find the directory
    fs_dir_t* dir = fs_find_dir(path);
    if (!dir) return FS_ERR_NOT_FOUND;
    
    // Can't remove root directory
    if (dir == fs->root) return FS_ERR_PERMISSION;
    
    // Directory must be empty
    if (dir->file_count > 0 || dir->subdir_count > 0) {
        return FS_ERR_PERMISSION;
    }
    
    // Find the parent directory
    fs_dir_t* parent = dir->parent;
    if (!parent) return FS_ERR_INVALID;
    
    // Remove directory from parent's list
    for (int i = 0; i < parent->subdir_count; i++) {
        if (parent->subdirs[i] == dir) {
            // Shift remaining directories
            for (int j = i; j < parent->subdir_count - 1; j++) {
                parent->subdirs[j] = parent->subdirs[j + 1];
            }
            parent->subdir_count--;
            
            // Free the directory
            free(dir);
            
            return FS_SUCCESS;
        }
    }
    
    return FS_ERR_NOT_FOUND;
}

// Change current directory
int fs_chdir(const char* path) {
    if (!fs || !path) return FS_ERR_INVALID;
    
    // Find the directory
    fs_dir_t* dir = fs_find_dir(path);
    if (!dir) return FS_ERR_NOT_FOUND;
    
    // Change current directory
    fs->current_dir = dir;
    
    return FS_SUCCESS;
}

// Get current working directory
char* fs_getcwd(char* buf, size_t size) {
    if (!fs || !buf || size == 0) return NULL;
    
    // Start with the current directory
    fs_dir_t* dir = fs->current_dir;
    
    // Special case for root directory
    if (dir == fs->root) {
        strncpy(buf, "/", size);
        return buf;
    }
    
    // Build path from bottom up
    char temp[FS_MAX_PATH_LENGTH] = "";
    while (dir != fs->root) {
        // Prepend directory name with slash
        char name_with_slash[FS_MAX_FILENAME_LENGTH + 1];
        snprintf(name_with_slash, sizeof(name_with_slash), "/%s", dir->name);
        
        // Prepend to temp path
        char temp_copy[FS_MAX_PATH_LENGTH];
        strncpy(temp_copy, temp, sizeof(temp_copy));
        snprintf(temp, sizeof(temp), "%s%s", name_with_slash, temp_copy);
        
        // Move up to parent
        dir = dir->parent;
    }
    
    // If path is empty, we're at root
    if (temp[0] == '\0') {
        strncpy(buf, "/", size);
    } else {
        strncpy(buf, temp, size);
    }
    
    return buf;
}

// List directory contents
int fs_listdir(const char* path, char* buffer, size_t bufsize) {
    if (!fs || !buffer || bufsize == 0) return FS_ERR_INVALID;
    
    // Find the directory
    fs_dir_t* dir = path ? fs_find_dir(path) : fs->current_dir;
    if (!dir) return FS_ERR_NOT_FOUND;
    
    // Clear the buffer
    buffer[0] = '\0';
    
    // Build the directory listing
    size_t offset = 0;
    
    // Add subdirectories
    for (int i = 0; i < dir->subdir_count; i++) {
        size_t len = strlen(dir->subdirs[i]->name);
        if (offset + len + 3 < bufsize) {
            offset += snprintf(buffer + offset, bufsize - offset, "%s/\n", dir->subdirs[i]->name);
        }
    }
    
    // Add files
    for (int i = 0; i < dir->file_count; i++) {
        size_t len = strlen(dir->files[i]->name);
        if (offset + len + 2 < bufsize) {
            offset += snprintf(buffer + offset, bufsize - offset, "%s\n", dir->files[i]->name);
        }
    }
    
    return FS_SUCCESS;
}

// Create a file
int fs_create(const char* path) {
    if (!fs || !path) return FS_ERR_INVALID;
    
    char normalized_path[FS_MAX_PATH_LENGTH];
    fs_normalize_path(path, normalized_path, sizeof(normalized_path));
    
    // Extract directory path and filename
    char dir_path[FS_MAX_PATH_LENGTH];
    char filename[FS_MAX_FILENAME_LENGTH];
    
    // Find the last slash in the path
    char* last_slash = strrchr(normalized_path, '/');
    if (last_slash) {
        // Extract directory path
        int dir_len = last_slash - normalized_path;
        strncpy(dir_path, normalized_path, dir_len);
        dir_path[dir_len] = '\0';
        
        // If dir_path is empty, use root
        if (dir_path[0] == '\0') {
            strcpy(dir_path, "/");
        }
        
        // Extract filename
        strcpy(filename, last_slash + 1);
    } else {
        // No slash, assume file is in current directory
        strcpy(dir_path, ".");
        strcpy(filename, normalized_path);
    }
    
    // Find the directory
    fs_dir_t* dir = fs_find_dir(dir_path);
    if (!dir) return FS_ERR_NOT_FOUND;
    
    // Check if file already exists
    for (int i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, filename) == 0) {
            return FS_ERR_EXISTS;
        }
    }
    
    // Check if name is already used by a directory
    for (int i = 0; i < dir->subdir_count; i++) {
        if (strcmp(dir->subdirs[i]->name, filename) == 0) {
            return FS_ERR_EXISTS;
        }
    }
    
    // Check if we have room for another file
    if (dir->file_count >= FS_MAX_FILES) {
        return FS_ERR_NO_SPACE;
    }
    
    // Create the new file
    fs_file_t* new_file = (fs_file_t*)malloc(sizeof(fs_file_t));
    if (!new_file) return FS_ERR_NO_SPACE;
    
    // Initialize the new file
    memset(new_file, 0, sizeof(fs_file_t));
    strncpy(new_file->name, filename, FS_MAX_FILENAME_LENGTH - 1);
    new_file->type = FS_TYPE_FILE;
    new_file->permissions = FS_PERM_READ | FS_PERM_WRITE;
    new_file->size = 0;
    new_file->data = NULL;
    new_file->created = get_time();
    new_file->modified = new_file->created;
    new_file->parent = dir;
    
    // Add the new file to the directory
    dir->files[dir->file_count++] = new_file;
    
    return FS_SUCCESS;
}

// Delete a file
int fs_delete(const char* path) {
    if (!fs || !path) return FS_ERR_INVALID;
    
    // Find the file
    fs_file_t* file = fs_find_file(path);
    if (!file) return FS_ERR_NOT_FOUND;
    
    // Get the parent directory
    fs_dir_t* dir = file->parent;
    if (!dir) return FS_ERR_INVALID;
    
    // Remove file from directory's list
    for (int i = 0; i < dir->file_count; i++) {
        if (dir->files[i] == file) {
            // Free file data if it exists
            if (file->data) {
                free(file->data);
            }
            
            // Free the file
            free(file);
            
            // Shift remaining files
            for (int j = i; j < dir->file_count - 1; j++) {
                dir->files[j] = dir->files[j + 1];
            }
            dir->file_count--;
            
            return FS_SUCCESS;
        }
    }
    
    return FS_ERR_NOT_FOUND;
}

// Read from a file
int fs_read(const char* path, void* buffer, size_t size, size_t offset) {
    if (!fs || !path || !buffer) return FS_ERR_INVALID;
    
    // Find the file
    fs_file_t* file = fs_find_file(path);
    if (!file) return FS_ERR_NOT_FOUND;
    
    // Check if file is readable
    if (!(file->permissions & FS_PERM_READ)) {
        return FS_ERR_PERMISSION;
    }
    
    // Check if offset is beyond file size
    if (offset >= file->size) {
        return 0;
    }
    
    // Calculate how much data we can read
    size_t read_size = size;
    if (offset + read_size > file->size) {
        read_size = file->size - offset;
    }
    
    // Copy data to buffer
    if (read_size > 0 && file->data) {
        memcpy(buffer, (uint8_t*)file->data + offset, read_size);
    }
    
    return read_size;
}

// Write to a file
int fs_write(const char* path, const void* buffer, size_t size, size_t offset) {
    if (!fs || !path || !buffer) return FS_ERR_INVALID;
    
    // Find the file
    fs_file_t* file = fs_find_file(path);
    if (!file) return FS_ERR_NOT_FOUND;
    
    // Check if file is writable
    if (!(file->permissions & FS_PERM_WRITE)) {
        return FS_ERR_PERMISSION;
    }
    
    // Check if we need to grow the file
    size_t new_size = offset + size;
    if (new_size > file->size) {
        // Check if new size exceeds maximum
        if (new_size > FS_MAX_FILE_SIZE) {
            return FS_ERR_NO_SPACE;
        }
        
        // Allocate or reallocate data buffer
        void* new_data = file->data ? realloc(file->data, new_size) : malloc(new_size);
        if (!new_data) {
            return FS_ERR_NO_SPACE;
        }
        
        // If we're extending the file, initialize new area to zeros
        if (offset > file->size) {
            memset((uint8_t*)new_data + file->size, 0, offset - file->size);
        }
        
        file->data = new_data;
        file->size = new_size;
    }
    
    // Copy data from buffer
    memcpy((uint8_t*)file->data + offset, buffer, size);
    
    // Update modification time
    file->modified = get_time();
    
    return size;
}

// Get file size
int fs_getsize(const char* path) {
    if (!fs || !path) return FS_ERR_INVALID;
    
    // Find the file
    fs_file_t* file = fs_find_file(path);
    if (!file) return FS_ERR_NOT_FOUND;
    
    return file->size;
}

// Check if a file or directory exists
int fs_exists(const char* path) {
    if (!fs || !path) return 0;
    
    // Try to find as directory first
    fs_dir_t* dir = fs_find_dir(path);
    if (dir) return 1;
    
    // Then try to find as file
    fs_file_t* file = fs_find_file(path);
    if (file) return 1;
    
    return 0;
}

// Create some example files and directories
void fs_create_example_files(void) {
    // Create some directories
    fs_mkdir("/bin");
    fs_mkdir("/etc");
    fs_mkdir("/home");
    fs_mkdir("/home/user");
    
    // Create some files
    fs_create("/etc/motd");
    fs_create("/home/user/hello.txt");
    
    // Write content to files
    const char* motd = "Welcome to MyOS!\nThis is a simple in-memory filesystem.\n";
    fs_write("/etc/motd", motd, strlen(motd), 0);
    
    const char* hello = "Hello, world!\nThis is a test file.\n";
    fs_write("/home/user/hello.txt", hello, strlen(hello), 0);
}
