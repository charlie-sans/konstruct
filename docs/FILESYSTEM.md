# konstruct Filesystem Documentation

This document describes the filesystem implementation in konstruct, detailing its architecture, supported features, and usage.

## Table of Contents

- [Overview](#overview)
- [Filesystem Architecture](#filesystem-architecture)
- [File Operations](#file-operations)
- [Directory Operations](#directory-operations)
- [Mount System](#mount-system)
- [Boot Device Detection](#boot-device-detection)
- [Using the Filesystem](#using-the-filesystem)
- [Implementation Details](#implementation-details)

## Overview

konstruct implements a simple filesystem that supports basic file and directory operations. The filesystem provides:

- File creation, reading, writing, and deletion
- Directory creation, listing, and navigation
- Support for different device types (CD-ROM, HDD, USB)
- Path-based access to files and directories

## Filesystem Architecture

The filesystem in konstruct follows a hierarchical structure similar to Unix-like systems:

```
/
├── cdrom/           # Mount point for CD-ROM device
├── media/           # Mount point for other storage devices
└── ...              # Other directories
```

### Key Components

1. **Filesystem Interface** (`fs/fs.c`): Provides a unified API for filesystem operations
2. **Boot Device Handler** (`fs/bootdev.c`): Manages detection and access to the boot device
3. **Mount System** (`fs/mount.c`): Handles mounting different filesystems to the directory tree

## File Operations

The following file operations are supported:

| Function | Description |
|----------|-------------|
| `fs_create(path)` | Creates a new empty file |
| `fs_delete(path)` | Deletes a file |
| `fs_read(path, buffer, size, offset)` | Reads data from a file |
| `fs_write(path, buffer, size, offset)` | Writes data to a file |
| `fs_getsize(path)` | Gets the size of a file |
| `fs_exists(path)` | Checks if a file exists |

## Directory Operations

The following directory operations are supported:

| Function | Description |
|----------|-------------|
| `fs_mkdir(path)` | Creates a new directory |
| `fs_listdir(path, buffer, buffer_size)` | Lists contents of a directory |
| `fs_chdir(path)` | Changes current working directory |
| `fs_getcwd(buffer, buffer_size)` | Gets current working directory |

## Mount System

The mount system allows different storage devices to be attached to the filesystem hierarchy. Key functions include:

| Function | Description |
|----------|-------------|
| `fs_mount(device, path, type)` | Mounts a device to a path |
| `fs_unmount(path)` | Unmounts a filesystem |
| `fs_remount(path)` | Remounts a device to a different path |

## Boot Device Detection

On startup, konstruct attempts to detect the boot device type (CD-ROM, Hard Drive, USB) and automatically mounts it at the appropriate location:

- CD-ROM devices are mounted at `/cdrom`
- Hard drives and USB devices are mounted at `/media`

## Using the Filesystem

### From the Shell

The shell provides commands to interact with the filesystem:

```
ls [path]     - List directory contents
cd [path]     - Change current directory
pwd           - Print working directory
mkdir path    - Create a directory
touch path    - Create an empty file
rm path       - Delete a file
cat path      - Display file contents
echo text > file - Write text to file
```

### From C Code

```c
#include <fs/fs.h>

// Example: Reading a file
int size = fs_getsize("/cdrom/myfile.txt");
if (size > 0) {
    char* buffer = malloc(size + 1);
    if (buffer) {
        int bytes_read = fs_read("/cdrom/myfile.txt", buffer, size, 0);
        buffer[bytes_read] = '\0';
        printf("File contents: %s\n", buffer);
        free(buffer);
    }
}

// Example: Writing to a file
const char* data = "Hello, world!\n";
fs_write("/media/output.txt", data, strlen(data), 0);
```

## Implementation Details

### File Representation

Files are stored as a simple block of data with metadata including:

- Name
- Size
- Creation time
- Modification time
- Access permissions

### Directory Representation

Directories are stored as lists of entries, each containing:

- Name
- Type (file or directory)
- Reference to the file or subdirectory

### Path Resolution

Paths are resolved by:

1. Checking if the path is absolute (starts with `/`) or relative
2. For absolute paths, starting at the root directory
3. For relative paths, starting at the current working directory
4. Traversing the path components one by one

### Limitations

The current implementation has some limitations:

- Limited file size (implementation dependent)
- No concurrent access protection
- Limited support for file attributes
- No journaling or crash recovery
- No support for hard links or symbolic links

## Future Improvements

Planned enhancements to the filesystem include:

- Support for more storage device types
- Improved error handling and reporting
- Support for file permissions and ownership
- Support for symbolic links
- Implementation of a disk-based filesystem format for persistent storage
