//unit8_t
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
// font type
typedef struct {
    int width;          // Width of the font
    int height;         // Height of the font
    int first_char;     // First character in the font
    int last_char;      // Last character in the font
    const uint8_t* data; // Font data
} Font;

// Include our libc
#include "libc/libc.h"

// Include VGA driver
#include "drivers/vga.h"

// Include filesystem
#include "fs/fs.h"

// Include enhanced terminal
#include "drivers/terminal.h"

// Multiboot header
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002, // Magic number
    0x0,        // Flags (no video mode, no additional info)
    -(0x1BADB002 + 0x0) // Checksum (magic + flags + checksum = 0)
};

// Define constants for video memory and colors
#define VIDEO_MEMORY 0xb8000
#define WHITE_ON_BLACK 0x0f

// Default screen dimensions (80x25 text mode)
#define DEFAULT_WIDTH 80
#define DEFAULT_HEIGHT 25

// VBE mode constants
#define VBE_CONTROLLER_INFO 0x4F00
#define VBE_MODE_INFO 0x4F01
#define VBE_SET_MODE 0x4F02

// Shell buffer size
#define CMD_BUFFER_SIZE 256

// Keyboard port definitions
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Global screen dimensions (can be changed)
int screen_width = DEFAULT_WIDTH;
int screen_height = DEFAULT_HEIGHT;
int is_graphics_mode = 0;  // 0 for text mode, 1 for graphics mode

// External declarations for font-related functions
extern const Font* font_get_default(void);
extern int font_get_pixel(const Font* font, char c, int x, int y);
extern void font_init(void);
extern int font_draw_char(const Font* font, int x, int y, char c, uint32_t color, uint32_t bgcolor);
extern int font_draw_string(const Font* font, int x, int y, const char* str, uint32_t color, uint32_t bgcolor);

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
    print_string("Welcome to MyOS with VGA Graphics, BMP Fonts, and Filesystem!\n");
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

// Function to clear the screen
void clear_screen(void) {
    char* video_memory = (char*)VIDEO_MEMORY;

    // Clear the entire screen based on current dimensions
    for (int i = 0; i < screen_width * screen_height * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = WHITE_ON_BLACK;
    }

    // Reset cursor position and update hardware cursor
    update_cursor(0, 0);
}

// Update the hardware cursor position
void update_cursor(int x, int y) {
    unsigned short position = y * screen_width + x;

    // Tell the VGA controller we are setting the cursor
    outb(0x3D4, 14);  // High byte
    outb(0x3D5, position >> 8);
    outb(0x3D4, 15);  // Low byte
    outb(0x3D5, position & 0xFF);
}

// Function to print a single character
void print_char(char c) {
    static int cursor_x = 0, cursor_y = 0;
    char* video_memory = (char*)VIDEO_MEMORY;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            int offset = (cursor_y * screen_width + cursor_x) * 2;
            video_memory[offset] = ' ';
            video_memory[offset + 1] = WHITE_ON_BLACK;
        }
    } else {
        int offset = (cursor_y * screen_width + cursor_x) * 2;
        video_memory[offset] = c;
        video_memory[offset + 1] = WHITE_ON_BLACK;
        cursor_x++;
        if (cursor_x >= screen_width) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= screen_height) {
        // Scroll the screen up by one line
        for (int i = 0; i < (screen_height - 1) * screen_width * 2; i++) {
            video_memory[i] = video_memory[i + screen_width * 2];
        }

        // Clear the last line
        for (int i = (screen_height - 1) * screen_width * 2; 
             i < screen_height * screen_width * 2; i += 2) {
            video_memory[i] = ' ';
            video_memory[i + 1] = WHITE_ON_BLACK;
        }

        // Move cursor up one row
        cursor_y = screen_height - 1;
    }

    // Update the hardware cursor
    update_cursor(cursor_x, cursor_y);
}

// Kernel-level putchar implementation
void kernel_putchar(char c) {
    print_char(c); // Use the existing print_char function
}

// Function to print a string
void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

// Function to read a scan code from the keyboard
unsigned char read_scan_code(void) {
    // Wait for a key to be pressed
    while (!(inb(KEYBOARD_STATUS_PORT) & 1));
    return inb(KEYBOARD_DATA_PORT);
}

// Function to convert a scan code to ASCII
char scancode_to_ascii(unsigned char scancode) {
    const char scancode_to_ascii_map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };

    // Special keys - handle Enter key explicitly
    if (scancode == 0x1C) { // Enter key scancode
        return '\n';
    }
    
    // Handle backspace
    if (scancode == 0x0E) { // Backspace key scancode
        return '\b';
    }

    if (scancode < sizeof(scancode_to_ascii_map)) {
        return scancode_to_ascii_map[scancode];
    }
    return 0; // Not a printable character
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
        print_string("Switching back to text mode...\n");
        
        // Direct mode setting for text mode
        if (vga_set_mode(VGA_MODE_TEXT_80x25)) {
            // We successfully switched to text mode
            is_graphics_mode = 0;
            screen_width = 80;
            screen_height = 25;
            
            // Print a welcome message in text mode
            clear_screen();
            print_string("Successfully switched to text mode.\n");
            return 1;
        } else {
            // Try the direct VGA register approach as fallback
            print_string("Failed to switch to text mode using standard method. Trying direct register approach...\n");
            
            // Reset to text mode using direct register access
            outb(0x3C2, 0x67);  // Miscellaneous Output register
            
            // Update our state variables
            vga_screen.width = 80;
            vga_screen.height = 25;
            vga_screen.bpp = 4;
            vga_screen.framebuffer = (void*)0xB8000;
            vga_screen.mode = VGA_MODE_TEXT_80x25;
            vga_screen.text_mode = 1;
            
            is_graphics_mode = 0;
            screen_width = 80;
            screen_height = 25;
            
            clear_screen();
            print_string("Forcefully switched to text mode.\n");
            return 1;
        }
    }
    
    // Check for standard VGA mode - 320x200
    if (width == 320 && height == 200) {
        print_string("Switching to 320x200 graphics mode...\n");

        if (vga_set_mode(VGA_MODE_320x200)) {
            // The terminal is automatically initialized in vga_set_mode
            
            // Set terminal colors
            vga_terminal_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
            
            // Draw a header
            vga_draw_filled_rect(0, 0, 320, 16, VGA_COLOR_DARK_GRAY);
            font_draw_string(font_get_default(), 4, 4, 
                             "MyOS Graphics Terminal", 
                             VGA_COLOR_WHITE, VGA_COLOR_DARK_GRAY);
            
            // Welcome message
            vga_terminal_write("\n Welcome to the graphics terminal!\n");
            vga_terminal_write(" Type 'help' for commands, or 'text' to return to text mode.\n\n");
            
            return 1;
        } else {
            print_string("Failed to switch to graphics mode!\n");
            return 0;
        }
    }
    
    // For higher resolution modes in QEMU, we'll simulate them with Mode 13h (320x200)
    // as QEMU often doesn't support all VBE modes properly
    else if ((width == 640 && height == 480) || 
             (width == 800 && height == 600) || 
             (width == 1024 && height == 768)) {
        
        char resolution_str[32];
        sprintf(resolution_str, "%dx%d", width, height);
        print_string("Attempting to switch to ");
        print_string(resolution_str);
        print_string(" mode...\n");
        
        // In QEMU, we'll simulate this by scaling our 320x200 mode
        if (vga_set_mode(VGA_MODE_320x200)) {
            // Pretend we're at a higher resolution for display purposes
            vga_draw_filled_rect(0, 0, 320, 16, VGA_COLOR_DARK_GRAY);
            font_draw_string(font_get_default(), 4, 4, 
                            resolution_str, 
                            VGA_COLOR_WHITE, VGA_COLOR_DARK_GRAY);
            font_draw_string(font_get_default(), 120, 4, 
                            "Simulation", 
                            VGA_COLOR_YELLOW, VGA_COLOR_DARK_GRAY);
            
            // Initialize the graphics terminal
            vga_terminal_init();
            
            // Welcome message
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
    
    // For other resolutions, use the simulated approach
    screen_width = width / 8;
    screen_height = height / 16;
    
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
    // Wait until the keyboard controller is ready
    while (inb(0x64) & 0x02);

    // Send the reset command to the keyboard controller
    outb(0x64, 0xFE);

    // If the reset command fails, halt the CPU
    while (1) {
        __asm__ __volatile__("hlt");
    }
}

// Perform a soft reboot using BIOS INT 19h
void soft_reboot(void) {
    // Trigger the bootstrap interrupt
    __asm__ __volatile__("int $0x19");
}

// Handle shell commands
void handle_command(const char* cmd) {
    // Check for graphics commands
    if (strcmp(cmd, "graphics") == 0) {
        console_write("Attempting to switch to 320x200 graphics mode...\n");
        set_vbe_mode(320, 200, 8);  // Switch to 320x200 graphics mode
        return;
    }
    else if (strcmp(cmd, "text") == 0) {
        console_write("Switching back to text mode...\n");
        set_vbe_mode(80, 25, 4);    // Switch back to text mode
        return;
    }
    else if (strcmp(cmd, "forcetext") == 0) {
        console_write("Forcing switch to text mode using direct VGA registers...\n");
        
        // Direct VGA register approach
        outb(0x3C2, 0x67);  // Miscellaneous Output register
        
        // Reset sequencer
        outb(0x3C4, 0x00);  // Index 0
        outb(0x3C5, 0x03);  // Reset sequencer
        
        // Set up sequencer for text mode
        outb(0x3C4, 0x01);  // Index 1: Clocking Mode
        outb(0x3C5, 0x00);  // 9 dot clock
        
        // Set up Graphics Controller for text mode
        outb(0x3CE, 0x05);  // Graphics mode register
        outb(0x3CF, 0x10);  // Set to text mode, odd/even addressing
        
        // End sequencer reset
        outb(0x3C4, 0x00);  // Index 0
        outb(0x3C5, 0x03);  // Clear reset bits
        
        // Update state variables
        vga_screen.width = 80;
        vga_screen.height = 25;
        vga_screen.bpp = 4;
        vga_screen.framebuffer = (void*)0xB8000;
        vga_screen.mode = VGA_MODE_TEXT_80x25;
        vga_screen.text_mode = 1;
        
        screen_width = 80;
        screen_height = 25;
        is_graphics_mode = 0;
        
        // Clear the screen and reset cursor
        {
            uint16_t* fb = (uint16_t*)0xB8000;
            for (int i = 0; i < 80 * 25; i++) {
                fb[i] = 0x0720; // White on black space
            }
        }
        
        // Clear screen and print welcome message
        clear_screen();
        print_string("Forced switch to text mode.\n");
        return;
    }
    else if (strcmp(cmd, "theme") == 0 || strncmp(cmd, "theme ", 6) == 0) {
        if (strlen(cmd) > 6) {
            // Apply the specified theme
            const char* theme_name = cmd + 6;
            terminal_load_theme(theme_name);
            terminal_print_status("Theme applied successfully");
        } else {
            // Show available themes
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
    // Filesystem commands
    else if (strncmp(cmd, "ls", 2) == 0 && (cmd[2] == '\0' || cmd[2] == ' ')) {
        // List directory contents
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
        // Change directory
        const char* path = cmd[2] == ' ' ? cmd + 3 : "/";
        
        int result = fs_chdir(path);
        if (result != FS_SUCCESS) {
            console_write("Error: Could not change directory.\n");
        }
    }
    else if (strncmp(cmd, "pwd", 3) == 0 && (cmd[3] == '\0' || cmd[3] == ' ')) {
        // Print working directory
        char cwd[FS_MAX_PATH_LENGTH] = {0};
        if (fs_getcwd(cwd, sizeof(cwd))) {
            console_write(cwd);
            console_write("\n");
        } else {
            console_write("Error: Could not get current directory.\n");
        }
    }
    else if (strncmp(cmd, "mkdir", 5) == 0 && cmd[5] == ' ') {
        // Create directory
        const char* path = cmd + 6;
        
        int result = fs_mkdir(path);
        if (result != FS_SUCCESS) {
            console_write("Error: Could not create directory.\n");
        }
    }
    else if (strncmp(cmd, "touch", 5) == 0 && cmd[5] == ' ') {
        // Create file
        const char* path = cmd + 6;
        
        int result = fs_create(path);
        if (result != FS_SUCCESS) {
            console_write("Error: Could not create file.\n");
        }
    }
    else if (strncmp(cmd, "rm", 2) == 0 && cmd[2] == ' ') {
        // Remove file
        const char* path = cmd + 3;
        
        int result = fs_delete(path);
        if (result != FS_SUCCESS) {
            console_write("Error: Could not delete file.\n");
        }
    }
    else if (strncmp(cmd, "cat", 3) == 0 && cmd[3] == ' ') {
        // Display file contents
        const char* path = cmd + 4;
        
        // Get file size
        int size = fs_getsize(path);
        if (size < 0) {
            console_write("Error: File not found.\n");
            return;
        }
        
        // Allocate buffer for file contents
        char* buffer = (char*)malloc(size + 1);
        if (!buffer) {
            console_write("Error: Out of memory.\n");
            return;
        }
        
        // Read file contents
        int bytes_read = fs_read(path, buffer, size, 0);
        if (bytes_read < 0) {
            console_write("Error: Could not read file.\n");
            free(buffer);
            return;
        }
        
        // Null-terminate and display
        buffer[bytes_read] = '\0';
        console_write(buffer);
        
        // Add newline if file doesn't end with one
        if (bytes_read > 0 && buffer[bytes_read - 1] != '\n') {
            console_write("\n");
        }
        
        free(buffer);
    }
    else if (strncmp(cmd, "echo", 4) == 0 && cmd[4] == ' ' && strstr(cmd, " > ")) {
        // Write to file using echo
        const char* content_start = cmd + 5;
        const char* redirect = strstr(cmd, " > ");
        
        if (!redirect) {
            console_write(content_start);
            console_write("\n");
            return;
        }
        
        // Extract content and filename
        int content_len = redirect - content_start;
        const char* filename = redirect + 3;
        
        char content[256];
        strncpy(content, content_start, content_len);
        content[content_len] = '\n';  // Add newline
        content[content_len + 1] = '\0';
        
        // Write to file (create if doesn't exist)
        if (!fs_exists(filename)) {
            fs_create(filename);
        }
        
        int result = fs_write(filename, content, strlen(content), 0);
        if (result < 0) {
            console_write("Error: Could not write to file.\n");
        }
    }
    // Check for resolution change command
    else if (strncmp(cmd, "resolution ", 11) == 0) {
        // Parse the resolution string (e.g., "resolution 1024x768")
        const char* res_str = cmd + 11;
        int width = 0, height = 0;
        
        // Simple parsing of "WIDTHxHEIGHT" format
        while (*res_str >= '0' && *res_str <= '9') {
            width = width * 10 + (*res_str - '0');
            res_str++;
        }
        
        if (*res_str == 'x' || *res_str == 'X') {
            res_str++;
            while (*res_str >= '0' && *res_str <= '9') {
                height = height * 10 + (*res_str - '0');
            }
            
            // Attempt to set the resolution
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
        // Switch to graphics mode and show font demo
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
    else if (strcmp(cmd, "help") == 0) {
        console_write("Available commands:\n");
        console_write("  help       - Show this help message\n");
        console_write("  clear      - Clear the screen\n");
        console_write("  version    - Show the OS version\n");
        console_write("  graphics   - Switch to 320x200 graphics mode\n");
        console_write("  640x480    - Switch to 640x480 graphics mode\n");
        console_write("  800x600    - Switch to 800x600 graphics mode\n");
        console_write("  1024x768   - Switch to 1024x768 graphics mode\n");
        console_write("  fontdemo   - Show a demonstration of BMP fonts\n");
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
    } 
    else if (strcmp(cmd, "version") == 0) {
        console_write("MyOS version 0.1 with integrated shell\n");
    } else {
        console_write("Unknown command: ");
        console_write(cmd);
        console_write("\n");
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
        
        handle_command(cmd_buffer);
    }
}
