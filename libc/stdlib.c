#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <drivers/serial.h>
#include "drivers/bios.h"
#include "../globals.h" // Add this include near the top of the file, with your other includes
#include "fs/fs.h"
#include "fs/bootdev.h"  // Add this include for bootdev functions

// Use the clear_screen function from stdio.c
extern void clear_screen(void);

// Function to read a scan code from the keyboard
unsigned char read_scan_code(void) {
    // Wait for a key to be pressed
    while (!(inb(KEYBOARD_STATUS_PORT) & 1));
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
}

// Function to set VBE mode
int set_vbe_mode(int width, int height, int bpp) {
    // Special case for text mode (80x25)
    if (width == 80 && height == 25) {
        console_write("Switching back to text mode...\n");
        serial_write_string(SERIAL_COM1_BASE, "Attempting to switch to text mode...\n");
        
        // Reset graphics mode flag first
        set_is_graphics_mode(0);
        set_screen_width(80);
        set_screen_height(25);
        
        // First try our new BIOS call method - most reliable
        set_text_mode();
        serial_write_string(SERIAL_COM1_BASE, "BIOS text mode call completed\n");
        
        // Update state variables
        vga_screen.width = 80;
        vga_screen.height = 25;
        vga_screen.bpp = 4;
        vga_screen.framebuffer = (void*)0xB8000;
        vga_screen.mode = VGA_MODE_TEXT_80x25;
        vga_screen.text_mode = 1;
        
        // Force reset all VGA registers to text mode values
        outb(0x3C2, 0x67);  // Miscellaneous Output register
        
        // Reset sequencer
        outb(0x3C4, 0x00);  // Index 0
        outb(0x3C5, 0x03);  // Reset sequencer
        
        // Set text mode sequencer values
        outb(0x3C4, 0x01);  // Clocking Mode register
        outb(0x3C5, 0x00);  // 9 dot clocks

        outb(0x3C4, 0x02);  // Map Mask register
        outb(0x3C5, 0x03);  // Enable planes 0 and 1
        
        outb(0x3C4, 0x03);  // Character Map Select register
        outb(0x3C5, 0x00);  // Default character set
        
        outb(0x3C4, 0x04);  // Memory Mode register
        outb(0x3C5, 0x02);  // Odd/Even addressing enabled
        
        // Set graphics controller for text mode
        outb(0x3CE, 0x05);  // Mode register
        outb(0x3CF, 0x10);  // Set to text mode, odd/even mode enabled
        
        outb(0x3CE, 0x06);  // Miscellaneous register
        outb(0x3CF, 0x0E);  // Text mode addressing
        
        clear_screen();
        print_string("Successfully switched to text mode.\n");
        serial_write_string(SERIAL_COM1_BASE, "Successfully switched to text mode.\n");
        
        return 1;
    }
    
    // Check for standard VGA mode - 320x200
    if (width == 320 && height == 200) {
        print_string("Switching to 320x200 graphics mode...\n");

        // First update the global variables
        set_is_graphics_mode(1);
        set_screen_width(width);
        set_screen_height(height);

        // Then try to set the mode
        if (vga_set_mode(VGA_MODE_320x200)) {
            vga_terminal_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
            vga_draw_filled_rect(0, 0, 320, 16, VGA_COLOR_DARK_GRAY);
            font_draw_string(font_get_default(), 4, 4, 
                             "konstruct Graphics Terminal", 
                             VGA_COLOR_WHITE, VGA_COLOR_DARK_GRAY);
            vga_terminal_write("\n Welcome to the graphics terminal!\n");
            vga_terminal_write(" Type 'help' for commands, or 'text' to return to text mode.\n\n");
            return 1;
        } else {
            // If failed, reset globals back to text mode
            set_is_graphics_mode(0);
            set_screen_width(80);
            set_screen_height(25);
            print_string("Failed to switch to graphics mode!\n");
            return 0;
        }
    }
    
    // For higher resolution modes in QEMU, we'll simulate them with Mode 13h (320x200)
    else if ((width == 640 && height == 480) || 
             (width == 800 && height == 600) || 
             (width == 1024 && height == 768)) {
        
        char resolution_str[32];
        sprintf(resolution_str, "%dx%d", width, height);
        print_string("Attempting to switch to ");
        print_string(resolution_str);
        print_string(" mode...\n");
        
        if (vga_set_mode(VGA_MODE_320x200)) {
            vga_draw_filled_rect(0, 0, 320, 16, VGA_COLOR_DARK_GRAY);
            font_draw_string(font_get_default(), 4, 4, 
                            resolution_str, 
                            VGA_COLOR_WHITE, VGA_COLOR_DARK_GRAY);
            font_draw_string(font_get_default(), 120, 4, 
                            "Simulation", 
                            VGA_COLOR_YELLOW, VGA_COLOR_DARK_GRAY);
            vga_terminal_init();
            vga_terminal_write("\n Simulating ");
            vga_terminal_write(resolution_str);
            vga_terminal_write(" mode\n");
            vga_terminal_write(" Note: In QEMU this is simulated using 320x200.\n\n");
            return 1;
        } else {
            print_string("Failed to set graphics mode.\n");
            return 0;
        }
    }
    
    print_string("Simulating resolution change to ");
    
    char buf[16];
    itoa(width, buf, 10);
    print_string(buf);
    print_string("x");
    itoa(height, buf, 10);
    print_string(buf);
    print_string("\n");
    
    clear_screen();
    
    return 1;
}

// Reboot the system using the keyboard controller
void reboot(void) {
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
    while (1) {
        __asm__ __volatile__("hlt");
    }
}

// Perform a soft reboot using BIOS INT 19h
void soft_reboot(void) {
    __asm__ __volatile__("int $0x19");
}

// Add new function to handle the 'run' command
void handle_run_command(const char* cmd) {
    const char* program_path = cmd + 4; // Skip "run "
    
    // Skip leading whitespace
    while (*program_path == ' ') program_path++;
    
    if (*program_path == '\0') {
        console_write("Usage: run <program_path>\n");
        return;
    }
    
    console_write("Attempting to run: ");
    console_write(program_path);
    console_write("\n");
    
    // Allocate buffer for program
    size_t buffer_size = 64 * 1024; // 64KB buffer
    void* program_buffer = malloc(buffer_size);
    
    if (!program_buffer) {
        console_write("Error: Failed to allocate memory for program\n");
        return;
    }
    
    // Load the program
    int result = fs_load_program(program_path, program_buffer, &buffer_size);
    
    if (result < 0) {
        console_write("Error: Failed to load program\n");
        free(program_buffer);
        return;
    }
    
    // Execute the program (this is a simplified example)
    console_write("Program loaded successfully. Executing...\n");
    
    typedef void (*program_entry_t)(void);
    program_entry_t entry_point = (program_entry_t)program_buffer;
    

    // Execute the program
    entry_point();
    console_write("Program execution not yet implemented\n");
    
    free(program_buffer);
}

// Handle shell commands
void handle_command(const char* cmd) {
    if (strcmp(cmd, "graphics") == 0) {
        console_write("Attempting to switch to 320x200 graphics mode...\n");
        set_vbe_mode(320, 200, 8);
        return;
    }
    else if (strcmp(cmd, "text") == 0) {
        console_write("Switching back to text mode...\n");
        set_vbe_mode(80, 25, 4);
        return;
    }
    else if (strcmp(cmd, "forcetext") == 0) {
        console_write("Forcing switch to text mode using direct VGA registers...\n");
        outb(0x3C2, 0x67);
        outb(0x3C4, 0x00);
        outb(0x3C5, 0x03);
        outb(0x3C4, 0x01);
        outb(0x3C5, 0x00);
        outb(0x3CE, 0x05);
        outb(0x3CF, 0x10);
        outb(0x3C4, 0x00);
        outb(0x3C5, 0x03);
        vga_screen.width = 80;
        vga_screen.height = 25;
        vga_screen.bpp = 4;
        vga_screen.framebuffer = (void*)0xB8000;
        vga_screen.mode = VGA_MODE_TEXT_80x25;
        vga_screen.text_mode = 1;
        {
            uint16_t* fb = (uint16_t*)0xB8000;
            for (int i = 0; i < 80 * 25; i++) {
                fb[i] = 0x0720;
            }
        }
        clear_screen();
        print_string("Forced switch to text mode.\n");
        return;
    }
    else if (strcmp(cmd, "theme") == 0 || strncmp(cmd, "theme ", 6) == 0) {
        if (strlen(cmd) > 6) {
            const char* theme_name = cmd + 6;
            terminal_load_theme(theme_name);
            terminal_print_status("Theme applied successfully");
        } else {
            terminal_println("Available themes:");
            terminal_println("  default - Blue and white theme");
            terminal_println("  dark    - Dark background with light text");
            terminal_println("  light   - Light background with dark text");
            terminal_println("  blue    - Blue gradient theme");
            terminal_println("  green   - Green console theme");
            terminal_println("  retro   - Classic green-on-black terminal");
            terminal_println("\nUsage: theme <name>");
        }
        return;
    }
    else if (strcmp(cmd, "clear") == 0) {
        terminal_clear_content();
        return;
    }
    else if (strncmp(cmd, "ls", 2) == 0 && (cmd[2] == '\0' || cmd[2] == ' ')) {
        char buffer[1024] = {0};
        const char* path = cmd[2] == ' ' ? cmd + 3 : NULL;
        int result = fs_listdir(path, buffer, sizeof(buffer));
        if (result == FS_SUCCESS) {
            console_write(buffer);
        } else {
            console_write("Error: Could not list directory.\n");
        }
    }
    else if (strncmp(cmd, "cd", 2) == 0 && (cmd[2] == '\0' || cmd[2] == ' ')) {
        const char* path = cmd[2] == ' ' ? cmd + 3 : "/";
        int result = fs_chdir(path);
        if (result != FS_SUCCESS) {
            console_write("Error: Could not change directory.\n");
        }
    }
    else if (strncmp(cmd, "pwd", 3) == 0 && (cmd[3] == '\0' || cmd[3] == ' ')) {
        char cwd[FS_MAX_PATH_LENGTH] = {0};
        if (fs_getcwd(cwd, sizeof(cwd))) {
            console_write(cwd);
            console_write("\n");
        } else {
            console_write("Error: Could not get current directory.\n");
        }
    }
    else if (strncmp(cmd, "mkdir", 5) == 0 && cmd[5] == ' ') {
        const char* path = cmd + 6;
        int result = fs_mkdir(path);
        if (result != FS_SUCCESS) {
            console_write("Error: Could not create directory.\n");
        }
    }
    else if (strncmp(cmd, "touch", 5) == 0 && cmd[5] == ' ') {
        const char* path = cmd + 6;
        int result = fs_create(path);
        if (result != FS_SUCCESS) {
            console_write("Error: Could not create file.\n");
        }
    }
    else if (strncmp(cmd, "rm", 2) == 0 && cmd[2] == ' ') {
        const char* path = cmd + 3;
        int result = fs_delete(path);
        if (result != FS_SUCCESS) {
            console_write("Error: Could not delete file.\n");
        }
    }
    else if (strncmp(cmd, "cat", 3) == 0 && cmd[3] == ' ') {
        const char* path = cmd + 4;
        int size = fs_getsize(path);
        if (size < 0) {
            console_write("Error: File not found.\n");
            return;
        }
        char* buffer = (char*)malloc(size + 1);
        if (!buffer) {
            console_write("Error: Out of memory.\n");
            return;
        }
        int bytes_read = fs_read(path, buffer, size, 0);
        if (bytes_read < 0) {
            console_write("Error: Could not read file.\n");
            free(buffer);
            return;
        }
        buffer[bytes_read] = '\0';
        console_write(buffer);
        if (bytes_read > 0 && buffer[bytes_read - 1] != '\n') {
            console_write("\n");
        }
        free(buffer);
    }
    else if (strncmp(cmd, "echo", 4) == 0 && cmd[4] == ' ' && strstr(cmd, " > ")) {
        const char* content_start = cmd + 5;
        const char* redirect = strstr(cmd, " > ");
        if (!redirect) {
            console_write(content_start);
            console_write("\n");
            return;
        }
        int content_len = redirect - content_start;
        const char* filename = redirect + 3;
        char content[256];
        strncpy(content, content_start, content_len);
        content[content_len] = '\n';
        content[content_len + 1] = '\0';
        if (!fs_exists(filename)) {
            fs_create(filename);
        }
        int result = fs_write(filename, content, strlen(content), 0);
        if (result < 0) {
            console_write("Error: Could not write to file.\n");
        }
    }
    else if (strncmp(cmd, "resolution ", 11) == 0) {
        const char* res_str = cmd + 11;
        int width = 0, height = 0;
        while (*res_str >= '0' && *res_str <= '9') {
            width = width * 10 + (*res_str - '0');
            res_str++;
        }
        if (*res_str == 'x' || *res_str == 'X') {
            res_str++;
            while (*res_str >= '0' && *res_str <= '9') {
                height = height * 10 + (*res_str - '0');
            }
            if (width > 0 && height > 0) {
                if (set_vbe_mode(width, height, 32)) {
                    return;
                } else {
                    console_write("Failed to set resolution.\n");
                    return;
                }
            }
        }
        console_write("Invalid resolution format. Use: resolution WIDTHxHEIGHT\n");
    } 
    else if (strcmp(cmd, "fontdemo") == 0) {
        console_write("Launching BMP font demo...\n");
        set_vbe_mode(320, 200, 8);
        return;
    }
    else if (strcmp(cmd, "640x480") == 0) {
        console_write("Switching to 640x480 graphics mode...\n");
        set_vbe_mode(640, 480, 8);
        return;
    }
    else if (strcmp(cmd, "800x600") == 0) {
        console_write("Switching to 800x600 graphics mode...\n");
        set_vbe_mode(800, 600, 8);
        return;
    }
    else if (strcmp(cmd, "1024x768") == 0) {
        console_write("Switching to 1024x768 graphics mode...\n");
        set_vbe_mode(1024, 768, 8);
        return;
    }
    else if (strncmp(cmd, "run", 3) == 0 && cmd[3] == ' ') {
        handle_run_command(cmd);
    }
    else if (strcmp(cmd, "mount") == 0) {
        console_write("Mounted filesystems:\n");
        if (bootdev_get_type() != BOOT_DEV_UNKNOWN) {
            char mount_info[100];
            sprintf(mount_info, "  %s on %s (%s)\n", 
                    bootdev_get_type() == BOOT_DEV_CDROM ? "/cdrom" : "/media",
                    bootdev_get_type_name(),
                    bootdev_get_type() == BOOT_DEV_CDROM ? "read-only" : "read-write");
            console_write(mount_info);
        } else {
            console_write("  No filesystems mounted\n");
        }
    }
    else if (strncmp(cmd, "mount ", 6) == 0) {
        console_write("Manual mounting not yet supported\n");
    }
    else if (strncmp(cmd, "remount ", 8) == 0) {
        const char* path = cmd + 8;
        int result = fs_remount(path);
        if (result == 0) {
            char buf[128];
            snprintf(buf, sizeof(buf), "Remounted boot device to %s\n", path);
            console_write(buf);
        } else {
            console_write("Failed to remount device\n");
        }
    }
    else if (strcmp(cmd, "help") == 0) {
        console_write("Available commands:\n");
        console_write("  help       - Show this help message\n");
        console_write("  clear      - Clear the screen\n");
        console_write("  version    - Show the OS version\n");
        console_write("  graphics   - Switch to 320x200 graphics mode\n");
        console_write("  640x480    - Switch to 640x480 graphics mode\n");
        console_write("  800x600    - Switch to 800x600 graphics mode\n");
        console_write("  mount      - Display mounted filesystems\n");
        console_write("  remount    - Remount boot device to a different path\n");
        console_write("  ls <path>  - List directory contents\n");
        console_write("  cat <file> - Display file contents\n");
        console_write("  run <prog> - Run a program from the filesystem\n");
        console_write("  text       - Switch back to text mode\n");
        console_write("  forcetext  - Force switch to text mode using direct VGA registers\n");
        console_write("  resolution - Change screen resolution (e.g., resolution 1024x768)\n");
        console_write("  theme      - Change terminal theme\n");
        console_write("  reboot     - Reboot the system\n");
        console_write("  softreboot - Perform a soft reboot\n");
        console_write("\nFilesystem commands:\n");
        console_write("  ls [path]  - List directory contents\n");
        console_write("  cd [path]  - Change current directory\n");
        console_write("  pwd        - Print working directory\n");
        console_write("  mkdir path - Create a directory\n");
        console_write("  touch path - Create an empty file\n");
        console_write("  rm path    - Delete a file\n");
        console_write("  cat path   - Display file contents\n");
        console_write("  echo text > file - Write text to file\n");
        console_write("\nMount commands:\n");
        console_write("  mount      - Show mounted filesystems\n");
        console_write("  mount dev  - Mount a specific device (not yet implemented)\n");
    } 
    else if (strcmp(cmd, "version") == 0) {
        console_write("konstruct version 0.1 with integrated shell\n");
    } else {
        console_write("Unknown command: ");
        console_write(cmd);
        console_write("\n");
    }
}