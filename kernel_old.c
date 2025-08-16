//unit8_// Include simple console driver
#include <drivers/simple_console.h>

// Include keyboard driver
#include <drivers/keyboard.h>

// Include filesystem
#include <fs/fs.h>

// Include enhanced terminal
#include <drivers/terminal.h>

// Include boot device handler
#include <fs/bootdev.h>gned char uint8_t;
typedef unsigned short uint16_t;

// Include our libc
#include <libc.h>
#include "debug.h"

// Include VGA driver
#include <drivers/vga.h>

// Include filesystem
#include <fs/fs.h>

// Include enhanced terminal
#include <drivers/terminal.h>

// Include boot device handler
#include <fs/bootdev.h>

// Include keyboard driver
#include <drivers/keyboard.h>

#include "kernel.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


// Define globals directly in kernel.c if linking isn't working

// Multiboot header
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002, // Magic number
    0x0,        // Flags (no video mode, no additional info)
    -(0x1BADB002 + 0x0) // Checksum (magic + flags + checksum = 0)
};
// External BMP font functions
extern void bmp_font_init(void);
extern Font* get_custom_bmp_font(void);
extern void bmp_font_cleanup(void);

// Function prototypes
void clear_screen(void);
void print_char(char c);
void print_string(const char* str);
void kernel_putchar(char c);
void shell_main(void);
void handle_command(const char* cmd);
unsigned char read_scan_code(void);
char scancode_to_ascii(unsigned char scancode);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void update_cursor(int x, int y);
int set_vbe_mode(int width, int height, int bpp);
void reboot(void);
void soft_reboot(void);
void handle_run_command(const char* cmd);

// Ensure this function is properly declared for external linking
char scancode_to_ascii(unsigned char scancode);

// Kernel main function
void kernel_main(void) {
    // Write directly to VGA memory as the very first thing
    char* vga = (char*)0xB8000;
    vga[0] = 'K'; vga[1] = 0x07;
    vga[2] = 'E'; vga[3] = 0x07;
    vga[4] = 'R'; vga[5] = 0x07;
    vga[6] = 'N'; vga[7] = 0x07;
    vga[8] = 'E'; vga[9] = 0x07;
    vga[10] = 'L'; vga[11] = 0x07;
    vga[12] = ' '; vga[13] = 0x07;
    vga[14] = 'S'; vga[15] = 0x07;
    vga[16] = 'T'; vga[17] = 0x07;
    vga[18] = 'A'; vga[19] = 0x07;
    vga[20] = 'R'; vga[21] = 0x07;
    vga[22] = 'T'; vga[23] = 0x07;
    
    // Initialize basic text mode only - no graphics
    simple_console_init();
    simple_console_printf("konstruct: Starting basic text mode...\n");
    
    // Initialize keyboard controller FIRST before anything else
    simple_console_printf("Initializing PS/2 keyboard...\n");
    
    // Add halt point before keyboard init
    simple_console_printf("DEBUG: About to call keyboard_init - press any key to continue\n");
    __asm__("hlt");  // Halt here to check if keyboard_init is the problem
    
    if (keyboard_init() != 0) {
        simple_console_printf("Warning: PS/2 keyboard initialization failed\n");
        // Halt on keyboard init failure to prevent restart
        simple_console_printf("HALTING: Keyboard init failed\n");
        while(1) __asm__("hlt");
    } else {
        simple_console_printf("PS/2 keyboard initialized successfully\n");
        // Halt after successful keyboard init to see if this is where restart happens
        simple_console_printf("DEBUG: Keyboard init successful - press any key to continue\n");
        __asm__("hlt");
    }
    
    // Skip VGA and font initialization for now - they might be causing conflicts
    simple_console_printf("Skipping VGA/font init to avoid conflicts...\n");
    
    // Initialize filesystem
    simple_console_printf("Initializing filesystem...\n");
    fs_init();
    simple_console_printf("Filesystem initialized\n");
    
    // Print a welcome message
    simple_console_printf("Welcome to konstruct - Basic Text Mode!\n");
    simple_console_printf("Type 'help' for a list of commands.\n");

    // Start a very basic shell loop
    simple_console_printf("Starting basic shell...\n");
    basic_shell_main();

    // Clean up resources
    fs_cleanup();

    // Halt the CPU
    while (1) {
        __asm__("hlt");
    }
}

// Basic shell function that avoids graphics conflicts
void basic_shell_main(void) {
    simple_console_printf("user@konstruct:$ ");
    
    // Very basic command loop with debugging
    while (1) {
        simple_console_printf("[DEBUG] About to call getchar\n");
        
        // Read a single character
        char c = (char)getchar();
        
        simple_console_printf("[DEBUG] getchar returned: ");
        if (c >= 32 && c <= 126) {
            simple_console_printf("%c", c);
        } else {
            simple_console_printf("(non-printable:%d)", (int)c);
        }
        simple_console_printf("\n");
        
        if (c == '\n' || c == '\r') {
            simple_console_printf("Command received!\n");
            simple_console_printf("user@konstruct:$ ");
        } else if (c != 0) {
            // Echo the character
            simple_console_printf("%c", c);
        }
    }
}


// Shell main function
void shell_main(void) {
    char cmd_buffer[CMD_BUFFER_SIZE];
    
    // Initialize enhanced terminal
    terminal_init();
    
    // Main command loop
    while (1) {
        // Display command prompt
        terminal_print_prompt();
        
        // Debug keyboard before reading input
        debug_print(DEBUG_INFO, "About to call terminal_readline\n");
        debug_keyboard_state();
        
        // Read user command
        terminal_readline(cmd_buffer, CMD_BUFFER_SIZE);
        
        // Skip empty commands
        if (cmd_buffer[0] == '\0') {
            continue;
        }
        
        // Handle the command
        if (strcmp(cmd_buffer, "reboot") == 0) {
            terminal_println("Rebooting...");
            reboot();
        } else if (strcmp(cmd_buffer, "softreboot") == 0) {
            terminal_println("Performing soft reboot...");
            soft_reboot();
        }
        else if (strcmp(cmd_buffer, "exit") == 0) {
            terminal_println("Exiting shell...");
            break; // Exit the shell loop
        } else if (strcmp(cmd_buffer, "help") == 0) {
    
        } else {
 
        }
        handle_command(cmd_buffer);
    }
}
