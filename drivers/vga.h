#ifndef VGA_H
#define VGA_H

#include "../libc/stdint.h"

// VGA mode constants
#define VGA_AC_INDEX         0x3C0
#define VGA_AC_WRITE         0x3C0
#define VGA_AC_READ          0x3C1
#define VGA_MISC_WRITE       0x3C2
#define VGA_SEQ_INDEX        0x3C4
#define VGA_SEQ_DATA         0x3C5
#define VGA_DAC_READ_INDEX   0x3C7
#define VGA_DAC_WRITE_INDEX  0x3C8
#define VGA_DAC_DATA         0x3C9
#define VGA_MISC_READ        0x3CC
#define VGA_GC_INDEX         0x3CE
#define VGA_GC_DATA          0x3CF
#define VGA_CRTC_INDEX       0x3D4
#define VGA_CRTC_DATA        0x3D5
#define VGA_INSTAT_READ      0x3DA

// Common VGA modes
#define VGA_MODE_TEXT_80x25  0x03  // Standard text mode
#define VGA_MODE_320x200     0x13  // 320x200 with 256 colors

// Add VESA VBE modes
#define VGA_MODE_640x480     0x101  // 640x480 with 256 colors
#define VGA_MODE_800x600     0x103  // 800x600 with 256 colors
#define VGA_MODE_1024x768    0x105  // 1024x768 with 256 colors

// Color constants for 256-color mode
#define VGA_COLOR_BLACK      0
#define VGA_COLOR_BLUE       1
#define VGA_COLOR_GREEN      2
#define VGA_COLOR_CYAN       3
#define VGA_COLOR_RED        4
#define VGA_COLOR_MAGENTA    5
#define VGA_COLOR_BROWN      6
#define VGA_COLOR_LIGHT_GRAY 7
#define VGA_COLOR_DARK_GRAY  8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_DARK_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED  12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW     14
#define VGA_COLOR_WHITE      15

// Additional constants for graphics terminal
#define VGA_GRAPHICS_COLS    40  // 320/8 = 40 columns in 320x200 mode
#define VGA_GRAPHICS_ROWS    25  // 200/8 = 25 rows in 320x200 mode

// Screen information structure
typedef struct {
    int width;       // Screen width in pixels
    int height;      // Screen height in pixels
    int bpp;         // Bits per pixel
    void* framebuffer; // Pointer to video memory
    int mode;        // Current video mode
    int text_mode;   // 1 if in text mode, 0 if in graphics mode
} vga_screen_t;

// VBE controller information structure
typedef struct {
    char signature[4];          // "VESA"
    uint16_t version;           // VBE version
    uint32_t oem_string_ptr;    // Pointer to OEM string
    uint32_t capabilities;      // Capabilities
    uint32_t video_modes_ptr;   // Pointer to video modes list
    uint16_t total_memory;      // Memory size in 64K blocks
} __attribute__((packed)) vbe_controller_info_t;

// VBE mode information structure
typedef struct {
    uint16_t attributes;        // Mode attributes
    uint8_t window_a;           // Window A attributes
    uint8_t window_b;           // Window B attributes
    uint16_t granularity;       // Window granularity
    uint16_t window_size;       // Window size
    uint16_t segment_a;         // Window A segment
    uint16_t segment_b;         // Window B segment
    uint32_t win_func_ptr;      // Pointer to window function
    uint16_t pitch;             // Bytes per scan line
    uint16_t width;             // Width in pixels
    uint16_t height;            // Height in pixels
    uint8_t w_char;             // Width of character cell
    uint8_t y_char;             // Height of character cell
    uint8_t planes;             // Number of memory planes
    uint8_t bpp;                // Bits per pixel
    uint8_t banks;              // Number of banks
    uint8_t memory_model;       // Memory model type
    uint8_t bank_size;          // Bank size in KB
    uint8_t image_pages;        // Number of images
    uint8_t reserved0;          // Reserved for page function
    
    // Direct color fields
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;
    
    // Linear framebuffer
    uint32_t framebuffer;       // Physical address of the linear framebuffer
    uint32_t off_screen_mem_off;// Offset of start of off screen memory
    uint16_t off_screen_mem_size;// Size of off screen memory in 1k units
    uint8_t reserved1[206];     // Remainder of ModeInfoBlock
} __attribute__((packed)) vbe_mode_info_t;

// Function prototypes
void vga_init(void);
int vga_set_mode(int mode);
void vga_clear_screen(uint8_t color);
void vga_putpixel(int x, int y, uint8_t color);
uint8_t vga_getpixel(int x, int y);
void vga_draw_line(int x1, int y1, int x2, int y2, uint8_t color);
void vga_draw_rect(int x, int y, int width, int height, uint8_t color);
void vga_draw_filled_rect(int x, int y, int width, int height, uint8_t color);
void vga_draw_circle(int x_center, int y_center, int radius, uint8_t color);
void vga_draw_char(int x, int y, char c, uint8_t color);
void vga_draw_string(int x, int y, const char* str, uint8_t color);

// Additional function prototypes for graphics text console
void vga_console_init(void);
void vga_console_clear(uint8_t color);
void vga_console_putchar(char c, uint8_t color);
void vga_console_print(const char* str, uint8_t color);
void vga_console_scroll(void);
void vga_reset_cursor(void);
void vga_update_cursor(int x, int y);

// Function to set VESA VBE mode
int vga_set_vbe_mode(uint16_t mode, vbe_mode_info_t* mode_info);

// Function to get available VBE modes
int vga_get_vbe_modes(uint16_t* modes, int max_modes);

// Function to get VBE controller information
int vga_get_vbe_controller_info(vbe_controller_info_t* info);

// Function to draw to higher resolution modes
void vga_draw_pixel_vbe(int x, int y, uint32_t color);

// Terminal functions
void vga_terminal_init(void);
void vga_terminal_cleanup(void);
void vga_terminal_putchar(char c);
void vga_terminal_write(const char* str);
void vga_terminal_set_color(uint8_t fg, uint8_t bg);
void console_putchar(char c);
void console_write(const char* str);

// Global screen information
extern vga_screen_t vga_screen;

#endif // VGA_H
