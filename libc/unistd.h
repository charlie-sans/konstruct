#ifndef UNISTD_H
#define UNISTD_H

#include "../libc/stdint.h"
#include "sys/stat.h"

// Standard file descriptors
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

typedef unsigned int size_t;

// System call prototypes
int _read(int fd, void* buf, size_t count);
int _write(int fd, const void* buf, size_t count);
int _open(const char* path, int flags, int mode);
int _close(int fd);
int _fstat(int fd, struct stat* st);
int _lseek(int fd, int offset, int whence);
void* _sbrk(int incr);
int _getpid(void);
int _kill(int pid, int sig);
void _exit(int status);

// Seek constants
#define SEEK_SET       0       // Set file offset to offset
#define SEEK_CUR       1       // Set file offset to current plus offset
#define SEEK_END       2       // Set file offset to EOF plus offset

#endif // UNISTD_H
