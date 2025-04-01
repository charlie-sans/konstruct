#include "libc/libc.h"

// function lookup table for us to call
// stores actual function pointers
typedef const char* (*function_ptr)(void);
typedef struct {
    const char* name;
    function_ptr func;
} function_lookup_t;


const char* help(void) {
    return "Available commands:\n"
           "  help - Show this help message\n"
           "  clear - Clear the screen\n"
           "  version - Show the OS version\n"
           "  echo - Echo the given text\n"
           "  mem - Test memory allocation\n"
           "  time - Show the current time\n";
}

//mem
const char* mem(void) {
    // Placeholder for memory test function
    return "Memory test function not implemented yet.";
}

const char* clear(void) {
    // Placeholder for clear function
    return "Clear function not implemented yet.";
}

const char* version(void) {
    return "MyOS version 0.1 with basic libc\n";
}

const char* echo(void) {
    // Placeholder for echo function
    return "Echo function not implemented yet.";
}

const char* time(void) {
    // Placeholder for time function
    return "Time function not implemented yet.";
}
function_lookup_t function_lookup[] = {
    {"help", help},
    {"clear", clear},
    {"version", version},
    {"echo", echo},
    {"mem", mem},
    {"time", time},
    {NULL, NULL}  // End of the list
};


void shell_main(void) {
    printf("Meow\n");
    printf("Type 'exit' to return to the kernel shell.\n");

    char input[256];
    while (1) {
        printf("meow> ");
        gets(input);

        if (strcmp(input, "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        int found = 0;
        for (int i = 0; function_lookup[i].name != NULL; i++) {
            if (strcmp(input, function_lookup[i].name) == 0) {
                const char* output = function_lookup[i].func();
                printf("%s\n", output);
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Unknown command: %s\n", input);
        }
    }
}
