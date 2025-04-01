#include "libc.h"  // Include standard library functions
#include "stdfont.h"  // Make sure stdfont.h with the BMP constants is included

// Define BMP constants directly if not already defined
#ifndef BMP_SIGNATURE
#define BMP_SIGNATURE      0x4D42   // 'BM' in little-endian
#endif
#ifndef BMP_HEADER_SIZE
#define BMP_HEADER_SIZE    14      // File header size
#endif
#ifndef BMP_INFO_SIZE
#define BMP_INFO_SIZE      40      // Info header size
#endif
#ifndef BMP_COMPRESSION_BI_RGB
#define BMP_COMPRESSION_BI_RGB 0   // No compression
#endif

// Check if the data represents a valid BMP file
int bmp_is_valid(const uint8_t* data, size_t size) {
    if (!data || size < (BMP_HEADER_SIZE + BMP_INFO_SIZE)) {
        return 0;
    }
    
    // Check BMP signature ('BM')
    uint16_t signature = data[0] | (data[1] << 8);
    if (signature != BMP_SIGNATURE) {
        return 0;
    }
    
    // Check file size matches or exceeds data size
    uint32_t fileSize = *(uint32_t*)(data + 2);
    if (fileSize > size) {
        return 0;
    }
    
    // Check header sizes
    uint32_t infoHeaderSize = *(uint32_t*)(data + BMP_HEADER_SIZE);
    if (infoHeaderSize < BMP_INFO_SIZE) {
        return 0;
    }
    
    return 1;
}

// Get the width of a BMP image
int bmp_get_width(const uint8_t* data) {
    if (!bmp_is_valid(data, BMP_HEADER_SIZE + BMP_INFO_SIZE + 8)) {
        return 0;
    }
    
    // Width is 4 bytes at offset 18 (BITMAPINFOHEADER)
    return *(int32_t*)(data + 18);
}

// Get the height of a BMP image
int bmp_get_height(const uint8_t* data) {
    if (!bmp_is_valid(data, BMP_HEADER_SIZE + BMP_INFO_SIZE + 12)) {
        return 0;
    }
    
    // Height is 4 bytes at offset 22 (BITMAPINFOHEADER)
    return *(int32_t*)(data + 22);
}

// Get the bits per pixel of a BMP image
int bmp_get_bpp(const uint8_t* data) {
    if (!bmp_is_valid(data, BMP_HEADER_SIZE + BMP_INFO_SIZE + 16)) {
        return 0;
    }
    
    // Bits per pixel is 2 bytes at offset 28 (BITMAPINFOHEADER)
    return *(uint16_t*)(data + 28);
}

// Helper: Calculate row padding for BMP
static int bmp_get_row_padding(int width, int bpp) {
    int bytesPerRow = (width * bpp + 7) / 8;
    return (4 - (bytesPerRow % 4)) % 4; // Rows are padded to 4-byte boundaries
}

// Get a pixel value from a BMP image
uint32_t bmp_get_pixel(const uint8_t* data, int x, int y) {
    if (!bmp_is_valid(data, BMP_HEADER_SIZE + BMP_INFO_SIZE + 16)) {
        return 0;
    }
    
    int width = bmp_get_width(data);
    int height = bmp_get_height(data);
    int bpp = bmp_get_bpp(data);
    
    // Bounds check
    if (x < 0 || x >= width || y < 0 || y >= abs(height)) {
        return 0;
    }
    
    // Get data offset
    uint32_t dataOffset = *(uint32_t*)(data + 10);
    
    // Handle y-flipping (BMP stores rows bottom-to-top by default)
    if (height > 0) {
        y = height - 1 - y;
    } else {
        height = -height; // Make height positive for calculations
    }
    
    // Calculate row padding
    int padding = bmp_get_row_padding(width, bpp);
    
    // Calculate pixel position
    const uint8_t* pixelData = data + dataOffset;
    
    switch (bpp) {
        case 1: {
            // 1 bit per pixel (monochrome)
            int byteOffset = y * ((width + 7) / 8 + padding) + (x / 8);
            int bitOffset = 7 - (x % 8); // MSB first
            return (pixelData[byteOffset] >> bitOffset) & 0x01;
        }
        
        case 4: {
            // 4 bits per pixel (16 colors)
            int byteOffset = y * ((width + 1) / 2 + padding) + (x / 2);
            if (x % 2 == 0) {
                return (pixelData[byteOffset] >> 4) & 0x0F;
            } else {
                return pixelData[byteOffset] & 0x0F;
            }
        }
        
        case 8: {
            // 8 bits per pixel (256 colors)
            int byteOffset = y * (width + padding) + x;
            return pixelData[byteOffset];
        }
        
        case 24: {
            // 24 bits per pixel (RGB)
            int byteOffset = y * (width * 3 + padding) + x * 3;
            return pixelData[byteOffset] | 
                  (pixelData[byteOffset + 1] << 8) | 
                  (pixelData[byteOffset + 2] << 16);
        }
        
        case 32: {
            // 32 bits per pixel (RGBA)
            int byteOffset = y * (width * 4) + x * 4;
            return *(uint32_t*)(pixelData + byteOffset);
        }
        
        default:
            return 0; // Unsupported bit depth
    }
}

// Create a font from a BMP image
Font* bmp_to_font(const uint8_t* bmp_data, int char_width, int char_height, 
                 char first_char, int chars_per_row, uint8_t* out_data, size_t out_size) {
    if (!bmp_is_valid(bmp_data, BMP_HEADER_SIZE + BMP_INFO_SIZE) || !out_data) {
        return NULL;
    }
    
    int width = bmp_get_width(bmp_data);
    int height = bmp_get_height(bmp_data);
    int bpp = bmp_get_bpp(bmp_data);
    
    // Validate parameters
    if (width <= 0 || height <= 0 || char_width <= 0 || char_height <= 0 || chars_per_row <= 0) {
        return NULL;
    }
    
    // Calculate number of characters in the font
    int chars_per_col = height / char_height;
    int total_chars = chars_per_row * chars_per_col;
    
    // Calculate required buffer size
    size_t required_size = total_chars * char_height;
    if (out_size < required_size) {
        return NULL; // Buffer too small
    }
    
    // Create a new font structure
    Font* font = (Font*)malloc(sizeof(Font));
    if (!font) {
        return NULL;
    }
    
    // Initialize font structure
    font->width = char_width;
    font->height = char_height;
    font->first_char = first_char;
    font->last_char = first_char + total_chars - 1;
    font->data = out_data;
    
    // Convert BMP to font data
    // For monochrome fonts: 1 byte per row, 8 bits per byte
    // Each character takes up char_height bytes
    for (int c = 0; c < total_chars; c++) {
        int row = c / chars_per_row;
        int col = c % chars_per_row;
        
        int bmp_x = col * char_width;
        int bmp_y = row * char_height;
        
        // Convert each character to font data
        for (int y = 0; y < char_height; y++) {
            uint8_t rowBits = 0;
            
            for (int x = 0; x < char_width; x++) {
                // Get pixel from BMP (treat non-zero as set)
                uint32_t pixel = bmp_get_pixel(bmp_data, bmp_x + x, bmp_y + y);
                
                // For 1-bit BMPs, use the bit value directly
                // For other formats, treat any non-zero/non-white color as set
                int bit = 0;
                if (bpp == 1) {
                    bit = pixel;
                } else {
                    // For color BMPs, use a threshold (not pure white)
                    bit = (pixel != 0 && pixel != 0xFFFFFF) ? 1 : 0;
                }
                
                // Set the appropriate bit in the current row
                if (bit) {
                    rowBits |= (1 << (7 - x));  // MSB first
                }
            }
            
            // Store the byte of bits for this row of the character
            out_data[c * char_height + y] = rowBits;
        }
    }
    
    return font;
}

// Function to load a font from a BMP file stored in memory
Font* load_font_from_bmp(const uint8_t* bmp_data, size_t bmp_size, 
                         int char_width, int char_height, char first_char) {
    // More detailed validation
    printf("Validating BMP data...\n");
    
    if (!bmp_is_valid(bmp_data, bmp_size)) {
        printf("Error: BMP data is not valid.\n");
        return NULL;
    }
    
    int width = bmp_get_width(bmp_data);
    int height = bmp_get_height(bmp_data);
    int bpp = bmp_get_bpp(bmp_data);
    
    printf("BMP dimensions: %dx%d, %d bpp\n", width, height, bpp);
    
    if (width <= 0 || height <= 0) {
        printf("Error: Invalid BMP dimensions.\n");
        return NULL;
    }
    
    if (char_width <= 0 || char_height <= 0) {
        printf("Error: Invalid character dimensions.\n");
        return NULL;
    }
    
    // Calculate characters per row based on BMP width
    int chars_per_row = width / char_width;
    if (chars_per_row <= 0) {
        printf("Error: Character width too large for BMP.\n");
        return NULL;
    }
    
    // Calculate size needed for font data
    int abs_height = abs(height);
    int chars_per_col = abs_height / char_height;
    
    if (chars_per_col <= 0) {
        printf("Error: Character height too large for BMP.\n");
        return NULL;
    }
    
    int total_chars = chars_per_row * chars_per_col;
    printf("Font will contain %d characters (%d per row, %d rows)\n", 
           total_chars, chars_per_row, chars_per_col);
    
    size_t font_data_size = total_chars * char_height;
    
    // Allocate memory for font data
    uint8_t* font_data = (uint8_t*)malloc(font_data_size);
    if (!font_data) {
        printf("Error: Failed to allocate %d bytes for font data.\n", font_data_size);
        return NULL;
    }
    
    // Initialize font data to zero
    memset(font_data, 0, font_data_size);
    
    // Convert BMP to font
    Font* font = bmp_to_font(bmp_data, char_width, char_height, 
                            first_char, chars_per_row, font_data, font_data_size);
    
    if (!font) {
        printf("Error: Failed to convert BMP to font.\n");
        free(font_data);
        return NULL;
    }
    
    printf("Font successfully created with %d characters\n", 
           font->last_char - font->first_char + 1);
    
    return font;
}
