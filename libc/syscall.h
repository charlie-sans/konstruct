#ifndef SYSCALL_H
#define SYSCALL_H
#include <sys/types.h>
// System call numbers
#define SYS_EXIT    1
#define SYS_FORK    2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETPID  7
#define SYS_SLEEP   8
#define SYS_KILL    9
#define SYS_EXECVE  10
#define SYS_WAITPID 11
#define SYS_BRKSYS  12
#define SYS_PUTCHAR 13
#define SYS_GETTIME 14
// Low-level syscall wrappers
int syscall0(int num);
int syscall1(int num, void* arg1);
int syscall2(int num, void* arg1, void* arg2);
int syscall3(int num, void* arg1, void* arg2, void* arg3);
int syscall4(int num, void* arg1, void* arg2, void* arg3, void* arg4);

// System call implementations
int sys_read(int fd, void* buf, int count);
int sys_write(int fd, const void* buf, int count);
int sys_open(const char* path, int flags);
int sys_close(int fd);
static int sys_exit(int status);

#endif // SYSCALL_H
