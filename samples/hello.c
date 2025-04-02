#include <libc.h>

// Entry point for user programs
int main(int argc, char** argv) {
    printf("Hello from user program!\n");
    
    if (argc > 1) {
        printf("Arguments:\n");
        for (int i = 1; i < argc; i++) {
            printf("  %d: %s\n", i, argv[i]);
        }
    }
    
    // Test memory allocation
    char* buffer = (char*)malloc(100);
    if (buffer) {
        puts("Memory allocation successful!");
        free(buffer);
    } else {
        puts("Memory allocation failed!");
    }
    
    return 42; // Return a test value
}
