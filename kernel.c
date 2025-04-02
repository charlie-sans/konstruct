//unit8_t
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

// Include our libc
#include <libc.h>

// Include VGA driver
#include <drivers/vga.h>

// Include filesystem
#include <fs/fs.h>

// Include enhanced terminal
#include <drivers/terminal.h>

// Include boot device handler
#include <fs/bootdev.h>

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
    // Initialize VGA subsystem
    vga_init();
    
    // Initialize font system
    font_init();
    
    // Initialize BMP font
    bmp_font_init();
    
    // Initialize filesystem
    fs_init();
    
    // Print a welcome message
    clear_screen();
    print_string("Welcome to konstruct with VGA Graphics, BMP Fonts, and Filesystem!\n");
    print_string("Type 'help' for a list of commands.\n");

    // Start the shell
    shell_main();

    // Clean up resources
    fs_cleanup();
    bmp_font_cleanup();

    // Halt the CPU
    while (1) {
        __asm__("hlt");
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
