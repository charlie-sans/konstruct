#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// External functions from your kernel
extern void kernel_putchar(char c);
extern void kernel_exit(int status);

// Heap boundaries (adjust as needed for your environment)
static char* heap_end = 0;
extern char _end; // Defined in your linker script
extern char _heap_end; // Defined in your linker script

// Write system call
ssize_t _write(int fd, const void* buf, size_t count) {
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO) {
        errno = EBADF;
        return -1;
    }

    const char* cbuf = (const char*)buf;
    for (size_t i = 0; i < count; i++) {
        kernel_putchar(cbuf[i]);
    }
    return count;
}

// Read system call (stubbed)
ssize_t _read(int fd, void* buf, size_t count) {
    errno = ENOSYS; // Not implemented
    return -1;
}

// sbrk system call (for dynamic memory allocation)
void* _sbrk(ptrdiff_t increment) {
    if (heap_end == 0) {
        heap_end = &_end; // Start of the heap
    }

    char* prev_heap_end = heap_end;
    if (heap_end + increment > &_heap_end) {
        errno = ENOMEM;
        return (void*)-1;
    }

    heap_end += increment;
    return prev_heap_end;
}

// Exit system call
void _exit(int status) {
    kernel_exit(status);
    while (1) {} // Halt
}

// Close system call (stubbed)
int _close(int fd) {
    errno = ENOSYS; // Not implemented
    return -1;
}

// fstat system call (stubbed)
int _fstat(int fd, struct stat* st) {
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        st->st_mode = S_IFCHR;
        return 0;
    }
    errno = EBADF;
    return -1;
}

// isatty system call
int _isatty(int fd) {
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        return 1;
    }
    errno = EBADF;
    return 0;
}

// lseek system call (stubbed)
off_t _lseek(int fd, off_t offset, int whence) {
    errno = ENOSYS; // Not implemented
    return -1;
}

// Open system call (stubbed)
int _open(const char* pathname, int flags, mode_t mode) {
    errno = ENOSYS; // Not implemented
    return -1;
}

// Kill system call (stubbed)
int _kill(pid_t pid, int sig) {
    errno = ENOSYS; // Not implemented
    return -1;
}

// Getpid system call (stubbed)
pid_t _getpid(void) {
    return 1; // Return a dummy PID
}
