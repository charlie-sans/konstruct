#ifndef _STDFONT_H
#define _STDFONT_H

#include "stdint.h"
#include "stddef.h"

// BMP file header structure
typedef struct {
    uint16_t signature;      // BMP signature, must be 'BM'
    uint32_t fileSize;       // Size of the BMP file in bytes
    uint32_t reserved;       // Reserved, must be 0
    uint32_t dataOffset;     // Offset to the start of image data
} __attribute__((packed)) BMPHeader;

// BMP info header structure
typedef struct {
    uint32_t headerSize;     // Size of this header (40 bytes)
    int32_t  width;          // Width of the image
    int32_t  height;         // Height of the image
    uint16_t planes;         // Number of color planes (must be 1)
    uint16_t bitsPerPixel;   // Bits per pixel
    uint32_t compression;    // Compression method
    uint32_t imageSize;      // Size of the image data
    int32_t  xPixelsPerM;    // Horizontal resolution
    int32_t  yPixelsPerM;    // Vertical resolution
    uint32_t colorsUsed;     // Number of colors in the palette
    uint32_t colorsImportant;// Number of important colors
} __attribute__((packed)) BMPInfoHeader;

// Font descriptor structure
typedef struct {
    uint8_t width;           // Character width in pixels
    uint8_t height;          // Character height in pixels
    uint8_t first_char;      // First character in the font (usually 32 for space)
    uint8_t last_char;       // Last character in the font
    const uint8_t* data;     // Pointer to the font bitmap data
} Font;

// Default 8x8 font data (will be filled with actual font data)
extern const uint8_t default_font_data[];
extern const Font default_font;

// Function prototypes

/**
 * Initialize font system
 */
void font_init(void);

/**
 * Get the default system font
 * 
 * @return Pointer to the default font
 */
const Font* font_get_default(void);

/**
 * Draw a character at the specified position
 * 
 * @param font The font to use
 * @param x X coordinate
 * @param y Y coordinate
 * @param c Character to draw
 * @param color Text color
 * @param bgcolor Background color
 * @return Width of the character drawn
 */
int font_draw_char(const Font* font, int x, int y, char c, uint32_t color, uint32_t bgcolor);

/**
 * Draw a string at the specified position
 * 
 * @param font The font to use
 * @param x X coordinate
 * @param y Y coordinate
 * @param str String to draw
 * @param color Text color
 * @param bgcolor Background color
 * @return Width of the string drawn
 */
int font_draw_string(const Font* font, int x, int y, const char* str, uint32_t color, uint32_t bgcolor);

/**
 * Calculate the width of a string if it were drawn
 * 
 * @param font The font to use
 * @param str String to measure
 * @return Width of the string in pixels
 */
int font_get_string_width(const Font* font, const char* str);

/**
 * Extract a font from a BMP image embedded in the executable
 * 
 * @param bmp_data Pointer to the BMP image data
 * @param font_out Pointer to Font structure to fill
 * @return 0 if successful, error code otherwise
 */
int font_from_bmp(const uint8_t* bmp_data, Font* font_out);

/**
 * Get the pixel value at the specified position in a bitmap font
 * 
 * @param font The font to use
 * @param c Character to check
 * @param x X position within the character (0 to width-1)
 * @param y Y position within the character (0 to height-1)
 * @return 1 if pixel is set, 0 if pixel is clear
 */
int font_get_pixel(const Font* font, char c, int x, int y);

// Example of including a font directly in the executable
// This would typically be defined in a .c file
#ifdef FONT_IMPLEMENTATION
// 8x8 default font data - Simple ASCII font
const uint8_t default_font_data[] = {
    // Space (32) - 8x8 bitmap
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ! (33) - 8x8 bitmap
    0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00,
    // " (34) - 8x8 bitmap
    0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ... and so on for all characters from 32 to 127
    // This would be replaced with actual font data
};

const Font default_font = {
    8,                 // width - 8 pixels per character
    8,                 // height - 8 pixels high
    32,                // first_char - starts at space (ASCII 32)
    127,               // last_char - ends at delete (ASCII 127)
    default_font_data  // data - points to our embedded font
};
#endif // FONT_IMPLEMENTATION

#endif // _STDFONT_H
