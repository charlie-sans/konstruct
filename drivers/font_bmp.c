#include "../libc/stdfont.h"
#include "../libc/libc.h"

// Create a simple monochrome BMP font data with at least A-Z, a-z, 0-9, and basic punctuation
// This is a minimal BMP file with a small font for testing
const uint8_t embedded_font_bmp[] = {
    // BMP Header (14 bytes)
    0x42, 0x4D,             // 'BM' signature
    0x76, 0x02, 0x00, 0x00, // File size
    0x00, 0x00, 0x00, 0x00, // Reserved
    0x76, 0x00, 0x00, 0x00, // Offset to pixel data
    
    // DIB Header (40 bytes)
    0x28, 0x00, 0x00, 0x00, // Header size
    0x40, 0x00, 0x00, 0x00, // Width (64 pixels)
    0x40, 0x00, 0x00, 0x00, // Height (64 pixels)
    0x01, 0x00,             // Planes (1)
    0x01, 0x00,             // Bits per pixel (1)
    0x00, 0x00, 0x00, 0x00, // Compression (none)
    0x00, 0x02, 0x00, 0x00, // Image size
    0x00, 0x00, 0x00, 0x00, // X pixels per meter
    0x00, 0x00, 0x00, 0x00, // Y pixels per meter
    0x02, 0x00, 0x00, 0x00, // Colors used (2)
    0x02, 0x00, 0x00, 0x00, // Important colors (2)
    
    // Color table (8 bytes)
    0x00, 0x00, 0x00, 0x00, // Color 0: Black
    0xFF, 0xFF, 0xFF, 0x00, // Color 1: White
    
    // Pixel data - an 8x8 grid of 8x8 characters (64x64 pixels)
    // Each character is encoded as 8 bytes (8 rows of 8 pixels each)
    // Each byte represents 8 pixels with most significant bit on the left
    
    // First row of characters (space ! " # $ % & ' ( ))
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x00, // !
    0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "
    0x14, 0x14, 0x3E, 0x14, 0x3E, 0x14, 0x14, 0x00, // #
    0x08, 0x3C, 0x0A, 0x1C, 0x28, 0x1E, 0x08, 0x00, // $
    0x06, 0x26, 0x10, 0x08, 0x04, 0x32, 0x30, 0x00, // %
    0x0C, 0x12, 0x0C, 0x1A, 0x24, 0x24, 0x1A, 0x00, // &
    0x0C, 0x0C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, // '
    
    // Second row of characters (* + , - . / 0 1)
    0x04, 0x08, 0x10, 0x10, 0x10, 0x08, 0x04, 0x00, // (
    0x10, 0x08, 0x04, 0x04, 0x04, 0x08, 0x10, 0x00, // )
    0x00, 0x14, 0x08, 0x3E, 0x08, 0x14, 0x00, 0x00, // *
    0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00, // +
    0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x08, 0x04, // ,
    0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, // -
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, // .
    0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00, // /
    

    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,

};

// Size of the embedded BMP data
const size_t embedded_font_bmp_size = sizeof(embedded_font_bmp);

// Global font pointer for loaded BMP font
Font* custom_bmp_font = NULL;

// Initialize the BMP font
void bmp_font_init(void) {
    // Allocate memory for font data
    size_t font_data_size = 96 * 8; // 96 characters, 8 bytes per character
    uint8_t* font_data = (uint8_t*)malloc(font_data_size);
    
    if (font_data) {
        // Validate BMP data
        if (!bmp_is_valid(embedded_font_bmp, embedded_font_bmp_size)) {
            printf("Error: Invalid BMP font data.\n");
            free(font_data);
            return;
        }

        // Convert BMP to font
        custom_bmp_font = bmp_to_font(
            embedded_font_bmp,      // BMP data
            8,                      // Character width
            8,                      // Character height
            32,                     // First character (space)
            8,                      // Characters per row
            font_data,              // Output buffer
            font_data_size          // Output buffer size
        );

        if (!custom_bmp_font) {
            printf("Error: Failed to convert BMP to font.\n");
            free(font_data);
        } else {
            printf("BMP font successfully loaded.\n");
        }
    } else {
        printf("Error: Failed to allocate memory for font data.\n");
    }
}

// Get the custom BMP font
Font* get_custom_bmp_font(void) {
    return custom_bmp_font;
}

// Clean up BMP font resources
void bmp_font_cleanup(void) {
    if (custom_bmp_font) {
        if (custom_bmp_font->data) {
            free((void*)custom_bmp_font->data);
        }
        free(custom_bmp_font);
        custom_bmp_font = NULL;
    }
}
