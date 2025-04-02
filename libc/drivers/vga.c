#include "vga.h"
#include "../libc/libc.h"
#include "../libc/stdfont.h"  // Add this include for font-related functions
#include "serial.h"
#include <stdlib.h>
#include <drivers/bios.h>
#include <globals.c>
// External functions from kernel.c
extern void outb(unsigned short port, unsigned char data);
extern unsigned char inb(unsigned short port);

// Global screen information
vga_screen_t vga_screen = {
    80, 25, 4, (void*)0xB8000, VGA_MODE_TEXT_80x25, 1
};

// Graphics mode console state
static int console_x = 0;
static int console_y = 0;
static uint8_t bg_color = VGA_COLOR_BLUE;

// Enhanced graphics terminal implementation
static int terminal_width = 1920;  // For 320x200 (80 chars with 8px font)
static int terminal_height = 1080; // For 320x200 (25 lines with 8px font)
static int terminal_bpp = 8;      // 256 colors (8 bits per pixel)
static char* terminal_buffer = NULL;
static uint8_t terminal_fg_color = VGA_COLOR_WHITE;
static uint8_t terminal_bg_color = VGA_COLOR_BLUE;

// Wait for a short time
static void vga_delay(void) {
    // Simple delay loop
    for (int i = 0; i < 10000; i++) {
        __asm__("nop");  // Do nothing operation
    }
}

// Initialize VGA subsystem
void vga_init(void) {
    // Default to text mode
    vga_screen.width = 80;
    vga_screen.height = 25;
    vga_screen.bpp = 4;
    vga_screen.framebuffer = (void*)0xB8000;
    vga_screen.mode = VGA_MODE_TEXT_80x25;
    vga_screen.text_mode = 1;
}

// Initialize better default VGA palette for mode 13h
static void set_default_palette(void) {
    // Standard 16-color palette (first 16 colors)
    static const uint8_t ega_colors[16][3] = {
        {0x00, 0x00, 0x00},    // 0: Black
        {0x00, 0x00, 0x2A},    // 1: Blue
        {0x00, 0x2A, 0x00},    // 2: Green
        {0x00, 0x2A, 0x2A},    // 3: Cyan
        {0x2A, 0x00, 0x00},    // 4: Red
        {0x2A, 0x00, 0x2A},    // 5: Magenta
        {0x2A, 0x15, 0x00},    // 6: Brown
        {0x2A, 0x2A, 0x2A},    // 7: Light Gray
        {0x15, 0x15, 0x15},    // 8: Dark Gray
        {0x15, 0x15, 0x3F},    // 9: Light Blue
        {0x15, 0x3F, 0x15},    // 10: Light Green
        {0x15, 0x3F, 0x3F},    // 11: Light Cyan
        {0x3F, 0x15, 0x15},    // 12: Light Red
        {0x3F, 0x15, 0x3F},    // 13: Light Magenta
        {0x3F, 0x3F, 0x15},    // 14: Yellow
        {0x3F, 0x3F, 0x3F}     // 15: White
    };

    // Set the first 16 colors to match the EGA palette
    for (int i = 0; i < 16; i++) {
        outb(VGA_DAC_WRITE_INDEX, i);
        outb(VGA_DAC_DATA, ega_colors[i][0]);
        outb(VGA_DAC_DATA, ega_colors[i][1]);
        outb(VGA_DAC_DATA, ega_colors[i][2]);
    }
    
    // Set grayscale for the rest of the palette
    for (int i = 16; i < 256; i++) {
        uint8_t gray = (i - 16) * 63 / 239;
        outb(VGA_DAC_WRITE_INDEX, i);
        outb(VGA_DAC_DATA, gray);
        outb(VGA_DAC_DATA, gray);
        outb(VGA_DAC_DATA, gray);
    }
}

// Set VGA Mode 13h (320x200 256 colors) using direct port access
static void set_mode_13h(void) {
    // Use int 0x10 as a primary method to set mode 13h
    regs16_t regs = {0};
    regs.ax = 0x0013;  // AH=0x00 (set video mode), AL=0x13 (graphics mode 320x200)
    vga_bios_call(&regs);
    
    // Short delay to allow mode change to complete
    vga_delay();
    
    // Reset the VGA controller
    outb(VGA_MISC_WRITE, 0x63);
    
    // Configure the sequencer registers
    outb(VGA_SEQ_INDEX, 0);    // Sequencer reset
    outb(VGA_SEQ_DATA, 0x03);  // Reset sequencer
    
    outb(VGA_SEQ_INDEX, 1);    // Clocking mode
    outb(VGA_SEQ_DATA, 0x01);  // 8 dot clock
    
    outb(VGA_SEQ_INDEX, 2);    // Map mask register
    outb(VGA_SEQ_DATA, 0x0F);  // Enable all planes
    
    outb(VGA_SEQ_INDEX, 3);    // Character map select
    outb(VGA_SEQ_DATA, 0x00);  // No character map
    
    outb(VGA_SEQ_INDEX, 4);    // Memory mode
    outb(VGA_SEQ_DATA, 0x0E);  // Chain-4 mode, extended memory
    
    // End sequencer reset
    outb(VGA_SEQ_INDEX, 0);
    outb(VGA_SEQ_DATA, 0x03);
    
    // Set Misc Output Register
    outb(VGA_MISC_WRITE, 0x63);
    
    // Unprotect CRT registers 0-7
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, 0x0);
    
    // Set CRT controller registers
    static const uint8_t crtc_regs[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF
    };
    
    for (int i = 0; i < sizeof(crtc_regs)/sizeof(crtc_regs[0]); i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, crtc_regs[i]);
    }
    
    // Set graphics controller registers
    static const uint8_t gc_regs[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF
    };
    
    for (int i = 0; i < sizeof(gc_regs)/sizeof(gc_regs[0]); i++) {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, gc_regs[i]);
    }
    
    // Set attribute controller registers
    static const uint8_t ac_regs[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00
    };
    
    for (int i = 0; i < sizeof(ac_regs)/sizeof(ac_regs[0]); i++) {
        inb(VGA_INSTAT_READ);  // Reset attribute controller flip-flop
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, ac_regs[i]);
    }
    
    // Enable video output
    inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
    
    // Use our improved palette instead of the default one
    set_default_palette();
    
    // Initialize console state
    console_x = 0;
    console_y = 0;
    
    // Update the global state
    set_is_graphics_mode(1);
}

// Set VGA Mode 3 (80x25 text mode) using direct port access
static void set_mode_3h(void) {
    // First try BIOS interrupt as a fallback method
    __asm__ volatile (
        "mov $0x03, %%ax\n"  // Text mode 3 (80x25 16 color)
        "int $0x10\n"        // BIOS video interrupt
        ::: "ax"
    );
    
    // Then continue with direct register programming
    
    // Reset the VGA controller to a known state first
    outb(0x3C2, 0x67);  // Set miscellaneous output register
    
    // Sequencer registers
    outb(0x3C4, 0x00);  // Sequencer index: Reset
    outb(0x3C5, 0x03);  // Reset sequencer
    
    outb(0x3C4, 0x01);  // Sequencer index: Clocking mode
    outb(0x3C5, 0x00);  // Normal operation
    
    outb(0x3C4, 0x02);  // Sequencer index: Map mask register
    outb(0x3C5, 0x03);  // Enable plane 0 & 1 (text mode planes)
    
    outb(0x3C4, 0x03);  // Sequencer index: Character map select
    outb(0x3C5, 0x00);  // Default character set
    
    outb(0x3C4, 0x04);  // Sequencer index: Memory mode
    outb(0x3C5, 0x02);  // Odd/Even addressing mode
    
    // Re-enable sequencer
    outb(0x3C4, 0x00);  // Sequencer index: Reset
    outb(0x3C5, 0x03);  // Clear reset bits
    
    // Make sure we set the correct CRT controller registers
    // Protect registers 0-7
    outb(0x3D4, 0x11);  // CRTC: Vertical retrace end register
    uint8_t value = inb(0x3D5) & 0x7F;  // Get current value and clear bit 7
    outb(0x3D5, value);  // Write new value to unprotect
    
    // Set text mode with explicit values
    static const uint8_t crtc_values[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
        0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3
    };
    
    // Write CRTC registers
    for (int i = 0; i < sizeof(crtc_values)/sizeof(crtc_values[0]); i++) {
        outb(0x3D4, i);         // Select register
        outb(0x3D5, crtc_values[i]); // Write value
    }
    
    // Explicitly set attribute controller registers for text mode
    static const uint8_t ac_regs[21] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x0C, 0x00, 0x0F, 0x08, 0x00
    };
    
    for (int i = 0; i < sizeof(ac_regs)/sizeof(ac_regs[0]); i++) {
        inb(VGA_INSTAT_READ);  // Reset attribute controller flip-flop
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, ac_regs[i]);
    }
    
    // Enable video output
    inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
    
    // Force cursor visible
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 0x0E);  // Cursor start
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 0x0F);  // Cursor end
    
    // Update pointer to framebuffer 
    uint16_t* fb = (uint16_t*)0xB8000;
    
    // Clear screen with default attributes (light gray on black)
    for (int i = 0; i < 80 * 25; i++) {
        fb[i] = 0x0720;  // White on black space
    }
    
    // Reset cursor position
    outb(0x3D4, 0x0F);  // CRTC: Cursor low byte
    outb(0x3D5, 0);
    outb(0x3D4, 0x0E);  // CRTC: Cursor high byte
    outb(0x3D5, 0);
    
    // Ensure we properly clean up any remaining graphics mode state
    if (terminal_buffer) {
        vga_terminal_cleanup();
    }
    
    // Log the mode change to serial port
    serial_write_string(SERIAL_COM1_BASE, "VGA Mode 3h (80x25 text mode) has been set\n");
}

// Set a specific VGA mode
int vga_set_mode(int mode) {
    // Clean up existing terminal if needed
    if (!vga_screen.text_mode && terminal_buffer) {
        vga_terminal_cleanup();
    }
    
    // Add some safety checks
    serial_write_string(SERIAL_COM1_BASE, "Setting VGA mode: ");
    char mode_str[8];
    int_to_string(mode, mode_str, 16);
    serial_write_string(SERIAL_COM1_BASE, mode_str);
    serial_write_string(SERIAL_COM1_BASE, "\n");

    switch (mode) {
        case VGA_MODE_TEXT_80x25:
            printf("Switching to VGA text mode (80x25).\n");
            // Update our screen information
            vga_screen.width = 80;
            vga_screen.height = 25;
            vga_screen.bpp = 4;
            vga_screen.framebuffer = (void*)0xB8000;
            vga_screen.mode = VGA_MODE_TEXT_80x25;
            vga_screen.text_mode = 1;
            
            // Set text mode using direct register manipulation
            set_mode_3h();
            
            // Update globals
            set_is_graphics_mode(0);
            set_screen_width(80);
            set_screen_height(25);
            
            return 1;
            
        case VGA_MODE_320x200:
            printf("Switching to VGA graphics mode (320x200).\n");
            // Update our screen information
            vga_screen.width = 320;
            vga_screen.height = 200;
            vga_screen.bpp = 8;
            vga_screen.framebuffer = (void*)0xA0000;
            vga_screen.mode = VGA_MODE_320x200;
            vga_screen.text_mode = 0;
            
            // Set graphics mode using direct register manipulation
            set_mode_13h();
            
            // Update globals
            set_is_graphics_mode(1);
            set_screen_width(320);
            set_screen_height(200);
            
            // Make sure the framebuffer is completely cleared
            {
                uint8_t* fb = (uint8_t*)vga_screen.framebuffer;
                for (int i = 0; i < 320 * 200; i++) {
                    fb[i] = 0; // Black
                }
            }
            
            // Initialize the graphics terminal
            vga_terminal_init();
            
            serial_write_string(SERIAL_COM1_BASE, "VGA Mode 13h initialized successfully\n");
            
            return 1;
        
        case VGA_MODE_640x480:
            printf("Switching to VGA graphics mode (640x480).\n");
            // Update our screen information
            vga_screen.width = 640;
            vga_screen.height = 480;
            vga_screen.bpp = 8;
            vga_screen.framebuffer = (void*)0xA0000;
            vga_screen.mode = VGA_MODE_640x480;
            vga_screen.text_mode = 0;
            
            // Set graphics mode using direct register manipulation
            set_mode_13h(); // Placeholder for actual 640x480 mode setting
            
            return 1;
        case VGA_MODE_800x600:
            printf("Switching to VGA graphics mode (800x600).\n");
            // Update our screen information
            vga_screen.width = 800;
            vga_screen.height = 600;
            vga_screen.bpp = 8;
            vga_screen.framebuffer = (void*)0xA0000;
            vga_screen.mode = VGA_MODE_800x600;
            vga_screen.text_mode = 0;
            
            // Set graphics mode using direct register manipulation
            set_mode_13h(); // Placeholder for actual 800x600 mode setting
            
            return 1;
        case VGA_MODE_1024x768:
            printf("Switching to VGA graphics mode (1024x768).\n");
            // Update our screen information
            vga_screen.width = 1024;
            vga_screen.height = 768;
            vga_screen.bpp = 8;
            vga_screen.framebuffer = (void*)0xA0000;
            vga_screen.mode = VGA_MODE_1024x768;
            vga_screen.text_mode = 0;
            
            // Set graphics mode using direct register manipulation
            set_mode_13h(); // Placeholder for actual 1024x768 mode setting
            
            return 1;
        default:
            printf("Error: Unsupported VGA mode %d.\n", mode);
            return 0; // Unsupported mode
    }
}

// Clear the screen with a specific color
void vga_clear_screen(uint8_t color) {
    if (vga_screen.text_mode) {
        // Text mode clear - fill with spaces
        uint16_t* fb = (uint16_t*)vga_screen.framebuffer;
        uint16_t value = 0x0700 | ' '; // White on black space
        
        for (int i = 0; i < vga_screen.width * vga_screen.height; i++) {
            fb[i] = value;
        }
    } else {
        // Graphics mode clear - fill with color
        uint8_t* fb = (uint8_t*)vga_screen.framebuffer;
        
        for (int i = 0; i < vga_screen.width * vga_screen.height; i++) {
            fb[i] = color;
        }
    }
}

// Set a single pixel in graphics mode
void vga_putpixel(int x, int y, uint8_t color) {
    if (vga_screen.text_mode || 
        x < 0 || x >= vga_screen.width || 
        y < 0 || y >= vga_screen.height) {
        return;
    }
    
    uint8_t* fb = (uint8_t*)vga_screen.framebuffer;
    fb[y * vga_screen.width + x] = color;
}

// Get a pixel color from the screen
uint8_t vga_getpixel(int x, int y) {
    if (vga_screen.text_mode || 
        x < 0 || x >= vga_screen.width || 
        y < 0 || y >= vga_screen.height) {
        return 0;
    }
    
    uint8_t* fb = (uint8_t*)vga_screen.framebuffer;
    return fb[y * vga_screen.width + x];
}

// Draw a line using Bresenham's algorithm
void vga_draw_line(int x1, int y1, int x2, int y2, uint8_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;
    
    while (1) {
        vga_putpixel(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) {
            break;
        }
        
        e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Draw an outlined rectangle
void vga_draw_rect(int x, int y, int width, int height, uint8_t color) {
    // Draw horizontal lines
    for (int i = 0; i < width; i++) {
        vga_putpixel(x + i, y, color);
        vga_putpixel(x + i, y + height - 1, color);
    }
    
    // Draw vertical lines
    for (int i = 0; i < height; i++) {
        vga_putpixel(x, y + i, color);
        vga_putpixel(x + width - 1, y + i, color);
    }
}

// Draw a filled rectangle
void vga_draw_filled_rect(int x, int y, int width, int height, uint8_t color) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            vga_putpixel(x + i, y + j, color);
        }
    }
}

// Draw a circle using midpoint circle algorithm
void vga_draw_circle(int x_center, int y_center, int radius, uint8_t color) {
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);
    
    while (x >= y) {
        vga_putpixel(x_center + x, y_center + y, color);
        vga_putpixel(x_center + y, y_center + x, color);
        vga_putpixel(x_center - y, y_center + x, color);
        vga_putpixel(x_center - x, y_center + y, color);
        vga_putpixel(x_center - x, y_center - y, color);
        vga_putpixel(x_center - y, y_center - x, color);
        vga_putpixel(x_center + y, y_center - x, color);
        vga_putpixel(x_center + x, y_center - y, color);
        
        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

// Draw a character in graphics mode
void vga_draw_char(int x, int y, char c, uint8_t color) {
    if (vga_screen.text_mode) {
        return;
    }
    
    // Get the default font from our font system
    const Font* font = font_get_default();
    
    // Make sure the character index is valid
    unsigned char ch = (unsigned char)c;
    // if (ch < font->first_char || ch > font->last_char) {
    //     // Replace unprintable characters with a visible placeholder
    //     if (ch != '\0' && ch != '\n' && ch != '\r' && ch != '\t' && ch != '\b') {
    //         ch = '.'; // Default fallback character
    //     } else {
    //         return; // Skip actual control characters
    //     }
    // }
    
    // Clear the character background area
    vga_draw_filled_rect(x, y, font->width, font->height, bg_color);
    
    // Draw the character pixel by pixel
    for (int j = 0; j < font->height; j++) {
        for (int i = 0; i < font->width; i++) {
            if (font_get_pixel(font, ch, i, j)) {
                vga_putpixel(x + i, y + j, color);
            }
        }
    }
}

// Draw a string in graphics mode
void vga_draw_string(int x, int y, const char* str, uint8_t color) {
    int offset_x = 0;
    
    while (*str) {
        vga_draw_char(x + offset_x, y, *str, color);
        offset_x += 8;  // Fixed 8-pixel font width
        str++;
    }
}

// Initialize console for graphics mode
void vga_console_init(void) {
    console_x = 0;
    console_y = 0;
    bg_color = VGA_COLOR_BLUE;
    vga_console_clear(bg_color);
    
    // Log to serial port
    serial_write_string(SERIAL_COM1_BASE, "Graphics console initialized\n");
}

// Clear the console in graphics mode
void vga_console_clear(uint8_t color) {
    bg_color = color;
    vga_clear_screen(color);
    console_x = 0;
    console_y = 0;
}

// Scroll the console up one line in graphics mode
void vga_console_scroll(void) {
    uint8_t* fb = (uint8_t*)vga_screen.framebuffer;
    int font_height = 8;
    
    // Move all rows up by one
    for (int y = 0; y < vga_screen.height - font_height; y++) {
        for (int x = 0; x < vga_screen.width; x++) {
            fb[y * vga_screen.width + x] = fb[(y + font_height) * vga_screen.width + x];
        }
    }
    
    // Clear the bottom row
    for (int y = vga_screen.height - font_height; y < vga_screen.height; y++) {
        for (int x = 0; x < vga_screen.width; x++) {
            fb[y * vga_screen.width + x] = bg_color;
        }
    }
    
    // Adjust console position
    console_y--;
}

// Update console character rendering to use BMP fonts when available
void vga_console_putchar(char c, uint8_t color) {
    int font_width = 8;
    int font_height = 8;
    
    // Handle newline
    if (c == '\n') {
        console_x = 0;
        console_y++;
    }
    // Handle carriage return
    else if (c == '\r') {
        console_x = 0;
    }
    // Handle backspace
    else if (c == '\b') {
        if (console_x > 0) {
            console_x--;
            // Draw a blank character at current position
            vga_draw_filled_rect(console_x * font_width, console_y * font_height, 
                                font_width, font_height, bg_color);
        }
    }
    // Handle tab
    else if (c == '\t') {
        int spaces = 4 - (console_x % 4);
        for (int i = 0; i < spaces; i++) {
            vga_console_putchar(' ', color);
        }
    }
    // Handle regular characters
    else {
        // Try to use BMP font if available
        Font* bmp_font = get_custom_bmp_font();
        if (bmp_font) {
            font_draw_char(bmp_font, 
                          console_x * font_width, 
                          console_y * font_height, 
                          c, color, bg_color);
        } else {
            // Fall back to built-in font
            vga_draw_char(console_x * font_width, 
                         console_y * font_height, 
                         c, color);
        }
        console_x++;
    }
    
    // Wrap text if we reach the end of a line
    if (console_x >= VGA_GRAPHICS_COLS) {
        console_x = 0;
        console_y++;
    }
    
    // Scroll if we reach the bottom of the screen
    if (console_y >= VGA_GRAPHICS_ROWS) {
        vga_console_scroll();
    }
}

// Print a string on the console in graphics mode
void vga_console_print(const char* str, uint8_t color) {
    while (*str) {
        vga_console_putchar(*str++, color);
    }
}

// Reset cursor to (0,0)
void vga_reset_cursor(void) {
    console_x = 0;
    console_y = 0;
}

// Update cursor position
void vga_update_cursor(int x, int y) {
    console_x = x;
    console_y = y;
}

// Initialize the graphics terminal
void vga_terminal_init(void) {
    // Calculate terminal dimensions based on screen resolution and font size
    terminal_width = vga_screen.width / 8;
    terminal_height = vga_screen.height / 8;
    
    // Allocate memory for the terminal buffer
    size_t buffer_size = terminal_width * terminal_height;
    terminal_buffer = (char*)malloc(buffer_size);
    
    if (terminal_buffer) {
        // Clear the terminal buffer
        memset(terminal_buffer, ' ', buffer_size);
        
        // Reset cursor position
        console_x = 0;
        console_y = 0;
        
        // Clear the screen
        vga_console_clear(terminal_bg_color);
    } else {
        printf("Failed to allocate terminal buffer!\n");
    }
}

// Clean up the graphics terminal
void vga_terminal_cleanup(void) {
    if (terminal_buffer) {
        free(terminal_buffer);
        terminal_buffer = NULL;
    }
}

// Write character to the terminal buffer and update the screen
void vga_terminal_putchar(char c) {
    if (!terminal_buffer) return;
    
    // Handle special characters
    if (c == '\n') {
        console_x = 0;
        console_y++;
    } else if (c == '\r') {
        console_x = 0;
    } else if (c == '\b') {
        if (console_x > 0) {
            console_x--;
            int offset = console_y * terminal_width + console_x;
            terminal_buffer[offset] = ' ';
            // Redraw the character
            font_draw_char(font_get_default(), 
                          console_x * 8, 
                          console_y * 8, 
                          ' ', 
                          terminal_fg_color, 
                          terminal_bg_color);
        }
    } else {
        // Normal character
        int offset = console_y * terminal_width + console_x;
        terminal_buffer[offset] = c;
        
        // Draw the character
        font_draw_char(font_get_default(), 
                      console_x * 8, 
                      console_y * 8, 
                      c, 
                      terminal_fg_color, 
                      terminal_bg_color);
        
        // Advance cursor
        console_x++;
    }
    
    // Handle line wrapping
    if (console_x >= terminal_width) {
        console_x = 0;
        console_y++;
    }
    
    // Handle scrolling
    if (console_y >= terminal_height) {
        // Move all lines up
        memmove(terminal_buffer, 
               terminal_buffer + terminal_width, 
               (terminal_height - 1) * terminal_width);
        
        // Clear the last line
        memset(terminal_buffer + (terminal_height - 1) * terminal_width, 
              ' ', 
              terminal_width);
        
        // Redraw the entire screen
        vga_console_clear(terminal_bg_color);
        for (int y = 0; y < terminal_height; y++) {
            for (int x = 0; x < terminal_width; x++) {
                char ch = terminal_buffer[y * terminal_width + x];
                font_draw_char(font_get_default(), 
                              x * 8, 
                              y * 8, 
                              ch, 
                              terminal_fg_color, 
                              terminal_bg_color);
            }
        }
        
        // Adjust cursor position
        console_y = terminal_height - 1;
    }
}

// Write string to the terminal
void vga_terminal_write(const char* str) {
    while (*str) {
        vga_terminal_putchar(*str++);
    }
}

// Set terminal colors
void vga_terminal_set_color(uint8_t fg, uint8_t bg) {
    terminal_fg_color = fg;
    terminal_bg_color = bg;
}

// Console putchar function that works in both text and graphics mode
void console_putchar(char c) {
    if (vga_screen.text_mode) {
        // Use traditional text mode functions
        print_char(c);
    } else {
        // Use graphics mode terminal
        vga_terminal_putchar(c);
    }
}

// Console write function that works in both text and graphics mode
void console_write(const char* str) {
    if (vga_screen.text_mode) {
        // Use traditional text mode functions
        print_string(str);
    } else {
        // Use graphics mode terminal
        vga_terminal_write(str);
    }
}
