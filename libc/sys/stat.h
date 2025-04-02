#ifndef SYS_STAT_H
#define SYS_STAT_H

#include "../../libc/stdint.h"

// File type and mode bits
#define S_IFMT  0170000   // Mask for file type bits
#define S_IFDIR 0040000   // Directory
#define S_IFCHR 0020000   // Character device
#define S_IFBLK 0060000   // Block device
#define S_IFREG 0100000   // Regular file
#define S_IFIFO 0010000   // FIFO
#define S_IFLNK 0120000   // Symbolic link

// File permissions
#define S_IRWXU 0000700   // RWX for owner
#define S_IRUSR 0000400   // Read for owner
#define S_IWUSR 0000200   // Write for owner
#define S_IXUSR 0000100   // Execute for owner
#define S_IRWXG 0000070   // RWX for group
#define S_IRWXO 0000007   // RWX for others

// Struct definition
struct stat {
    uint32_t st_dev;      // Device ID
    uint32_t st_ino;      // Inode number
    uint32_t st_mode;     // File type and mode
    uint32_t st_nlink;    // Number of hard links
    uint32_t st_uid;      // User ID of owner
    uint32_t st_gid;      // Group ID of owner
    uint32_t st_rdev;     // Device ID (if special file)
    uint32_t st_size;     // Total size in bytes
    uint32_t st_blksize;  // Block size for filesystem I/O
    uint32_t st_blocks;   // Number of 512B blocks allocated
    uint32_t st_atime;    // Time of last access
    uint32_t st_mtime;    // Time of last modification
    uint32_t st_ctime;    // Time of last status change
};

#endif // SYS_STAT_H
