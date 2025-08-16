#include "libc/drivers/simple_console.h"
#include "libc/drivers/keyboard.h"
#include "libc/fs/fs.h"
#include "libc/libc.h"

// Function prototypes
unsigned char read_scan_code(void);
char scancode_to_ascii(unsigned char scancode);

// Multiboot header
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002, // Magic number
    0x0,        // Flags (no video mode, no additional info)
    -(0x1BADB002 + 0x0) // Checksum (magic + flags + checksum = 0)
};

// Simple kernel entry point
void kernel_main(void) {
    // Write directly to VGA memory to confirm we're running
    char* vga = (char*)0xB8000;
    vga[0] = 'K'; vga[1] = 0x07;
    vga[2] = 'O'; vga[3] = 0x07;
    vga[4] = 'N'; vga[5] = 0x07;
    vga[6] = 'S'; vga[7] = 0x07;
    vga[8] = 'T'; vga[9] = 0x07;
    vga[10] = 'R'; vga[11] = 0x07;
    vga[12] = 'U'; vga[13] = 0x07;
    vga[14] = 'C'; vga[15] = 0x07;
    vga[16] = 'T'; vga[17] = 0x07;
    
    // Initialize console
    simple_console_init();
    simple_console_print("konstruct kernel starting...\n");
    
    // Initialize keyboard
    simple_console_print("Initializing keyboard...\n");
    keyboard_init();
    simple_console_print("Keyboard initialized\n");
    
    // Simple key echo test - just echo whatever key is pressed
    simple_console_print("Key echo test - press keys to see them echoed\n");
    simple_console_print("Press ESC to exit\n");
    
    while (1) {
        // Read scan code directly from keyboard
        unsigned char scancode = read_scan_code();
        
        if (scancode != 0) {
            // Only process key press events (not release)
            if (!(scancode & 0x80)) {
                // Convert to ASCII
                char ascii = scancode_to_ascii(scancode);
                
                if (ascii != 0) {
                    // Echo the character
                    simple_console_print("Got key: ");
                    simple_console_putchar(ascii);
                    simple_console_print(" (scancode: ");
                    
                    // Print scancode in hex
                    if (scancode < 16) simple_console_putchar('0');
                    char hex_chars[] = "0123456789ABCDEF";
                    simple_console_putchar(hex_chars[scancode >> 4]);
                    simple_console_putchar(hex_chars[scancode & 0xF]);
                    
                    simple_console_print(")\n");
                    
                    // Exit on ESC (scancode 1)
                    if (scancode == 1) {
                        simple_console_print("ESC pressed - exiting\n");
                        break;
                    }
                } else {
                    // Non-ASCII key
                    simple_console_print("Non-ASCII key (scancode: ");
                    if (scancode < 16) simple_console_putchar('0');
                    char hex_chars[] = "0123456789ABCDEF";
                    simple_console_putchar(hex_chars[scancode >> 4]);
                    simple_console_putchar(hex_chars[scancode & 0xF]);
                    simple_console_print(")\n");
                }
            }
        }
    }
}
