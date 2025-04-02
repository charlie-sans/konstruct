#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Basic font structure
typedef struct CustomFont {
    uint8_t width;          // Width of each character
    uint8_t height;         // Height of each character
    uint8_t spacing;        // Spacing between characters
    uint8_t* data;          // Font bitmap data
    uint32_t default_fg;    // Default foreground color
    uint32_t default_bg;    // Default background color
} Font;

// Function to get default font
extern const Font* font_get_default(void);
extern Font* get_custom_bmp_font(void);

// Function to get a pixel from the font
extern int font_get_pixel(const Font* font, char c, int x, int y);

#endif /* FONT_H */
