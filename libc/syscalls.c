#include "unistd.h"
#include "errno.h"


// Define global errno variable
int errno = 0;


extern void kernel_putchar(char c);
extern char read_scan_code(void);
extern char scancode_to_ascii(unsigned char scancode);

// Heap boundaries (adjust as needed for your environment)
static char* heap_end = 0;
extern char _end; // Defined in your linker script
#define HEAP_START ((char*)&_end)
#define HEAP_MAX   ((char*)0x1000000) // 16MB max heap

// Write system call
int _write(int fd, const void* buf, size_t count) {
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

// Read system call
int _read(int fd, void* buf, size_t count) {
    if (fd != STDIN_FILENO) {
        errno = EBADF;
        return -1;
    }

    char* cbuf = (char*)buf;
    size_t i;
    for (i = 0; i < count; i++) {
        unsigned char scancode;
        char c;
        
        // Wait for a key press
        scancode = read_scan_code();
        c = scancode_to_ascii(scancode);
        
        // Check for special characters
        if (c == '\n') {
            cbuf[i] = c;
            i++;
            break;
        }
        
        if (c != 0) { // Skip non-printable characters
            cbuf[i] = c;
            // Echo the character if reading from stdin
            if (fd == STDIN_FILENO) {
                kernel_putchar(c);
            }
        } else {
            i--; // Retry for this position
        }
    }
    
    return i;
}

// sbrk system call (for dynamic memory allocation)
void* _sbrk(int incr) {
    char* prev_heap_end;
    
    if (heap_end == 0) {
        heap_end = HEAP_START;
    }
    
    prev_heap_end = heap_end;
    
    if (heap_end + incr > HEAP_MAX) {
        errno = ENOMEM;
        return (void*)-1;
    }
    
    heap_end += incr;
    return (void*)prev_heap_end;
}

// Open system call (stub)
int _open(const char* path, int flags, int mode) {
    errno = ENOSYS; // Not implemented
    return -1;
}

// Close system call (stub)
int _close(int fd) {
    // Ignore the fd parameter to avoid warning
    (void)fd;
    errno = ENOSYS; // Not implemented
    return -1;
}

// File status system call
int _fstat(int fd, struct stat* st) {
    // Ignore the fd parameter to avoid warning
    (void)fd;
    
    if (st != NULL) {
        // Set as character device so isatty() will return 1
        st->st_mode = S_IFCHR;
        return 0;
    }
    
    errno = EINVAL;
    return -1;
}

// Seek system call (stub)
int _lseek(int fd, int offset, int whence) {
    // Avoid unused parameter warnings
    (void)fd;
    (void)offset;
    (void)whence;
    
    errno = ESPIPE; // Invalid seek
    return -1;
}

// Process ID (stub)
int _getpid(void) {
    return 1; // Return a dummy PID
}

// Kill process (stub)
int _kill(int pid, int sig) {
    // Avoid unused parameter warnings
    (void)pid;
    (void)sig;
    
    errno = EINVAL;
    return -1;
}

// Exit process
void _exit(int status) {
    // Return to the caller with the exit status
    // In a real OS, this would terminate the process
    while (1) {
        // Halt CPU - in reality we'd switch back to the kernel
        __asm__("hlt");
    }
}

// Required for linking with GCC compiled programs
void __attribute__((weak)) __main(void) {
    // This is called by GCC-generated code before main
    // We provide an empty implementation
}
