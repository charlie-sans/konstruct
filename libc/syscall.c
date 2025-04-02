#include <syscall.h>
#include <libc.h>
#include <fs/fs.h>

// External references to kernel functions
extern void kernel_putchar(char c);
extern char read_scan_code(void);
extern char scancode_to_ascii(unsigned char scancode);

// Current program break (for memory allocation)
static char* program_break = (char*)0x800000; // 8MB mark

// System call handler functions
static int sys_exit(int status) {
    // In a real multi-process system, this would terminate the process
    // For now, just return to the caller with the exit status
    return status;
}

int sys_read(int fd, void* buf, int count) {
    // Currently only support stdin (fd=0)
    if (fd != 0) return -1;
    
    char* cbuf = (char*)buf;
    int i;
    for (i = 0; i < count; i++) {
        unsigned char scancode = read_scan_code();
        char c = scancode_to_ascii(scancode);
        if (c == '\n') {
            cbuf[i] = c;
            i++;
            break;
        }
        if (c != 0) { // Skip non-printable characters
            cbuf[i] = c;
        } else {
            i--; // Retry for this position
        }
    }
    return i;
}

int sys_write(int fd, const void* buf, int count) {
    // Currently only support stdout/stderr (fd=1/2)
    if (fd != 1 && fd != 2) return -1;
    
    const char* cbuf = (const char*)buf;
    for (int i = 0; i < count; i++) {
        kernel_putchar(cbuf[i]);
    }
    return count;
}

static int sys_brksys(void* addr) {
    // Simple memory allocation - no real bounds checking
    if (addr == NULL) {
        return (int)program_break;
    }
    
    char* new_break = (char*)addr;
    program_break = new_break;
    return (int)program_break;
}

// Main syscall dispatcher
int handle_syscall(int num, void* arg1, void* arg2, void* arg3, void* arg4) {
    switch (num) {
        case SYS_EXIT:
            return sys_exit((int)arg1);
        
        case SYS_READ:
            return sys_read((int)arg1, arg2, (int)arg3);
            
        case SYS_WRITE:
            return sys_write((int)arg1, arg2, (int)arg3);
            
        case SYS_BRKSYS:
            return sys_brksys(arg1);
            
        case SYS_PUTCHAR:
            kernel_putchar((char)(int)arg1);
            return 0;
            
        default:
            return -1; // Unsupported syscall
    }
}

// Syscall wrappers for user programs
int syscall0(int num) {
    int result;
    asm volatile(
        "int $0x80"
        : "=a"(result)
        : "a"(num)
        : "memory"
    );
    return result;
}

int syscall1(int num, void* arg1) {
    int result;
    asm volatile(
        "int $0x80"
        : "=a"(result)
        : "a"(num), "b"(arg1)
        : "memory"
    );
    return result;
}

int syscall2(int num, void* arg1, void* arg2) {
    int result;
    asm volatile(
        "int $0x80"
        : "=a"(result)
        : "a"(num), "b"(arg1), "c"(arg2)
        : "memory"
    );
    return result;
}

int syscall3(int num, void* arg1, void* arg2, void* arg3) {
    int result;
    asm volatile(
        "int $0x80"
        : "=a"(result)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
        : "memory"
    );
    return result;
}

int syscall4(int num, void* arg1, void* arg2, void* arg3, void* arg4) {
    int result;
    asm volatile(
        "int $0x80"
        : "=a"(result)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4)
        : "memory"
    );
    return result;
}

// Initialize syscall handling
void syscall_init(void) {
    // Set up interrupt handler for syscalls (int 0x80)
    // This would be implemented in your interrupt handling code
}
