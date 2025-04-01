/* Minimal kernel for our OS with shell functionality */

// Include our libc
#include "libc/libc.h"

// Define a constant for the video memory address
#define VIDEO_MEMORY 0xb8000
// Define colors
#define WHITE_ON_BLACK 0x0f
#define RED_ON_BLACK 0x04
#define GREEN_ON_BLACK 0x02

// Screen dimensions
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Keyboard port definitions
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Special key codes
#define KEY_ENTER 0x1C
#define KEY_BACKSPACE 0x0E
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_EXTENDED 0xE0

// Command buffer size and history settings
#define CMD_BUFFER_SIZE 256
#define HISTORY_SIZE 10

// Function prototypes for kernel-specific functions
void clear_screen(void);
void print_char(char c);
void handle_command(void);
void print_prompt(void);
char read_scan_code(void);
void update_cursor(void);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
char scancode_to_ascii(char scancode);
void add_to_history(const char* cmd);
void navigate_history(int direction);
void clear_command_line(void);
void set_command_line(const char* cmd);
void shell_main(void);

// Global variables
int cursor_x = 0;
int cursor_y = 0;
char cmd_buffer[CMD_BUFFER_SIZE];
int cmd_pos = 0;

// Command history
char history[HISTORY_SIZE][CMD_BUFFER_SIZE];
int history_count = 0;        // Number of commands in history
int history_position = -1;    // Current position in history when browsing
int history_index = 0;        // Index where next command will be stored

// Flag for extended key sequences
int extended_key = 0;

// Function attribute to ensure this is placed at the start of the binary
__attribute__((section(".text.start")))
// Kernel main function
void kernel_main(void) {
    // Clear the screen
    clear_screen();
    
    // Print a welcome message using our new libc functions
    printf("Welcome to MyOS v0.1!\n");
    printf("This OS now includes a basic libc implementation.\n");
    printf("Type 'help' for available commands.\n\n");
    
    // Initialize the command buffer and history
    memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
    for (int i = 0; i < HISTORY_SIZE; i++) {
        memset(history[i], 0, CMD_BUFFER_SIZE);
    }
    
    // Print the initial prompt
    print_prompt();

    printf("Starting shell...\n");
    shell_main();

    printf("Shell exited. Returning to kernel.\n");
    while (1) {
        // Halt the CPU
        __asm__("hlt");
    }
}

// Convert scan code to ASCII
char scancode_to_ascii(char scancode) {
    const char scancode_to_ascii_map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    
    if (scancode < sizeof(scancode_to_ascii_map)) {
        return scancode_to_ascii_map[scancode];
    }
    return 0;  // Not a printable character
}

// Read a scan code from the keyboard
char read_scan_code(void) {
    // Wait for a key to be pressed
    while (!(inb(KEYBOARD_STATUS_PORT) & 1));
    
    // Read the scan code
    return inb(KEYBOARD_DATA_PORT);
}

// Low-level port I/O functions
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
    return;
}

// Add a command to history
void add_to_history(const char* cmd) {
    // Don't add empty commands or duplicates of the last command
    if (cmd[0] == '\0' || (history_count > 0 && strcmp(cmd, history[(history_index + HISTORY_SIZE - 1) % HISTORY_SIZE]) == 0)) {
        return;
    }
    
    // Copy the command to the history buffer
    strcpy(history[history_index], cmd);
    
    // Update indexes
    history_index = (history_index + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE) {
        history_count++;
    }
}

// Navigate through command history
void navigate_history(int direction) {
    // direction: 1 for up (older), -1 for down (newer)
    if (direction > 0) {
        // Going up in history (older commands)
        if (history_position < history_count - 1) {
            history_position++;
            int index = (history_index + HISTORY_SIZE - 1 - history_position) % HISTORY_SIZE;
            set_command_line(history[index]);
        }
    } else {
        // Going down in history (newer commands)
        if (history_position > 0) {
            history_position--;
            int index = (history_index + HISTORY_SIZE - 1 - history_position) % HISTORY_SIZE;
            set_command_line(history[index]);
        } else if (history_position == 0) {
            // Return to empty command line when reaching bottom of history
            history_position = -1;
            clear_command_line();
        }
    }
}

// Clear the current command line
void clear_command_line(void) {
    // Calculate the prompt length (e.g., "MyOS> ")
    int prompt_len = 6;  // Length of "MyOS> "
    
    // Move cursor back to start of command
    cursor_x = prompt_len;
    
    // Clear the existing command text
    char* video_memory = (char*) VIDEO_MEMORY;
    for (int i = 0; i < cmd_pos; i++) {
        int offset = 2 * (cursor_y * SCREEN_WIDTH + cursor_x + i);
        video_memory[offset] = ' ';
        video_memory[offset + 1] = WHITE_ON_BLACK;
    }
    
    // Reset command buffer
    memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
    cmd_pos = 0;
    
    // Update cursor position
    update_cursor();
}

// Set the command line to a specific command
void set_command_line(const char* cmd) {
    // First clear the current command
    clear_command_line();
    
    // Then set the new command
    strcpy(cmd_buffer, cmd);
    cmd_pos = strlen(cmd);
    
    // Display the command
    print_string_internal(cmd);
}

// Handle a command
void handle_command(void) {
    if (cmd_buffer[0] == 0) {
        // Empty command, do nothing
        return;
    }
    
    // Compare against known commands
    if (strcmp(cmd_buffer, "help") == 0) {
        puts("Available commands:");
        puts("  help     - Display this help message");
        puts("  clear    - Clear the screen");
        puts("  version  - Display the OS version");
        puts("  echo     - Echo the given text");
        puts("  mem      - Test memory allocation");
    }
    else if (strcmp(cmd_buffer, "clear") == 0) {
        clear_screen();
        cursor_x = 0;
        cursor_y = 0;
    }
    else if (strcmp(cmd_buffer, "version") == 0) {
        puts("MyOS version 0.1 with basic libc");
    }
    else if (strncmp(cmd_buffer, "echo ", 5) == 0) {
        // Echo the text after "echo "
        puts(cmd_buffer + 5);
    }
    else if (strcmp(cmd_buffer, "mem") == 0) {
        // Test memory allocation
        char* mem1 = (char*)malloc(16);
        char* mem2 = (char*)malloc(32);
        
        if (mem1 && mem2) {
            strcpy(mem1, "Hello, ");
            strcpy(mem2, "world!");
            
            printf("Memory test: %s%s\n", mem1, mem2);
            printf("Memory addresses: mem1=%x, mem2=%x\n", (unsigned int)mem1, (unsigned int)mem2);
            
            free(mem1);
            free(mem2);
        } else {
            puts("Memory allocation failed!");
        }
    }
    else {
        printf("Unknown command: %s\n", cmd_buffer);
        puts("Type 'help' for available commands.");
    }
}

// Print the shell prompt
void print_prompt(void) {
    printf("MyOS> ");
}

// Update the hardware cursor position
void update_cursor(void) {
    unsigned short position = cursor_y * SCREEN_WIDTH + cursor_x;
    
    // Tell the VGA controller we are setting the cursor
    outb(0x3D4, 14);  // High byte
    outb(0x3D5, position >> 8);
    outb(0x3D4, 15);  // Low byte
    outb(0x3D5, position & 0xFF);
}

// Function to clear the screen
void clear_screen(void) {
    char* video_memory = (char*) VIDEO_MEMORY;
    
    // The screen has 25 rows and 80 columns
    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH * 2; i += 2) {
        video_memory[i] = ' ';  // Character
        video_memory[i+1] = WHITE_ON_BLACK; // Attribute
    }
    
    // Reset cursor position
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

// Function to print a string - rename to avoid conflicts
void print_string_internal(const char* string) {
    while(*string != 0) {
        print_char(*string);
        string++;
    }
}

// Function to print a single character
void print_char(char c) {
    char* video_memory = (char*) VIDEO_MEMORY;
    
    // Handle special characters
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } 
    else if (c == '\r') {
        cursor_x = 0;
    }
    else if (c == '\t') {
        // Tab is 4 spaces
        for (int i = 0; i < 4; i++) {
            print_char(' ');
        }
    }
    else {
        // Calculate the position in video memory
        int offset = 2 * (cursor_y * SCREEN_WIDTH + cursor_x);
        
        // Write the character
        video_memory[offset] = c;
        video_memory[offset + 1] = WHITE_ON_BLACK;
        
        // Update cursor position
        cursor_x++;
        if (cursor_x >= SCREEN_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    
    // Handle scrolling if we're past the bottom of the screen
    if (cursor_y >= SCREEN_HEIGHT) {
        // Scroll the screen up by one line
        for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i++) {
            video_memory[i] = video_memory[i + SCREEN_WIDTH * 2];
        }
        
        // Clear the last line
        for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; 
             i < SCREEN_HEIGHT * SCREEN_WIDTH * 2; i += 2) {
            video_memory[i] = ' ';
            video_memory[i + 1] = WHITE_ON_BLACK;
        }
        
        // Move cursor up one row
        cursor_y = SCREEN_HEIGHT - 1;
    }
    
    // Update the hardware cursor
    update_cursor();
}
