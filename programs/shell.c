#include "libc/libc.h"

void shell_main(void) {
    printf("Welcome to MyOS Shell!\n");
    printf("Type 'exit' to return to the kernel.\n");

    char input[256];
    while (1) {
        printf("shell> ");
        gets(input);

        if (strcmp(input, "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        } else if (strcmp(input, "help") == 0) {
            printf("Available commands:\n");
            printf("  help - Show this help message\n");
            printf("  exit - Exit the shell\n");
        } else {
            printf("Unknown command: %s\n", input);
        }
    }
}
