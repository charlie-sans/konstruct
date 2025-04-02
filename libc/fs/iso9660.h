#ifndef ISO9660_H
#define ISO9660_H

#include <stdint.h>
#include "bootdev.h"
#include "../libc/libc.h"
// Initialize the ISO9660 filesystem
int iso9660_init(boot_device_t* dev);

// Read a file from the ISO9660 filesystem
int iso9660_read_file(const char* path, void* buffer, size_t size);

// List a directory from the ISO9660 filesystem
int iso9660_list_directory(const char* path, char* buffer, size_t size);

#endif // ISO9660_H
