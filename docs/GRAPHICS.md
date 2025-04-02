# konstruct Graphics Subsystem Documentation

This document describes the graphics capabilities of konstruct, including text mode, VGA graphics, and custom font rendering.

## Table of Contents

- [Overview](#overview)
- [Display Modes](#display-modes)
- [Text Mode](#text-mode)
- [Graphics Modes](#graphics-modes)
- [Font Rendering](#font-rendering)
- [ANSI Escape Sequences](#ansi-escape-sequences)
- [Terminal Emulation](#terminal-emulation)
- [Code Examples](#code-examples)

## Overview

konstruct provides a flexible graphics subsystem that supports both text and graphics modes. The system can switch between modes at runtime and provides APIs for drawing text, shapes, and handling user interfaces.

## Display Modes

konstruct supports the following display modes:

| Mode | Resolution | Colors | Description |
|------|------------|--------|-------------|
| Text Mode | 80x25 characters | 16 foreground, 8 background | Standard VGA text mode |
| VGA Mode 13h | 320x200 pixels | 256 colors | Classic VGA graphics mode |
| VESA Modes | Various (e.g., 640x480, 800x600, 1024x768) | 256 to 16M colors | Extended graphics modes via VESA BIOS Extensions |

Mode switching is handled through BIOS calls and direct VGA register manipulation.

## Text Mode

Text mode provides a character-based display with 80 columns and 25 rows. Each character position can display an ASCII character with customizable foreground and background colors.

### Features

- Double-buffering for smooth updates
- Cursor positioning
- Color attributes (16 foreground colors, 8 background colors)
- Scrolling support
- ANSI escape sequence interpretation for terminal control

### Text Colors

Available colors in text mode:

| Color Code | Color Name |
|------------|------------|
| 0 | BLACK |
| 1 | BLUE |
| 2 | GREEN |
| 3 | CYAN |
| 4 | RED |
| 5 | MAGENTA |
| 6 | BROWN |
| 7 | LIGHT_GRAY |
| 8 | DARK_GRAY |
| 9 | LIGHT_BLUE |
| 10 | LIGHT_GREEN |
| 11 | LIGHT_CYAN |
| 12 | LIGHT_RED |
| 13 | LIGHT_MAGENTA |
| 14 | YELLOW |
| 15 | WHITE |

## Graphics Modes

Graphics modes provide pixel-based display with various resolutions and color depths.

### VGA Mode 13h (320x200)

This is the classic VGA graphics mode with 320x200 resolution and 256 colors. It uses a linear framebuffer at memory address 0xA0000.

### VESA Modes

konstruct supports higher resolution modes through VESA BIOS Extensions (VBE). Available resolutions depend on hardware support but typically include:

- 640x480
- 800x600
- 1024x768

### Drawing Functions

Graphics mode provides functions for:

- Setting individual pixels
- Drawing lines
- Drawing rectangles (filled and outlined)
- Drawing circles
- Bitblt operations (copying rectangular regions)
- Displaying bitmap images

## Font Rendering

konstruct includes a font rendering system that supports:

- Built-in bitmap fonts
- Custom fonts loaded from BMP files
- Variable width and height characters
- Color and background rendering

### Default Font

The system includes a default 8x8 bitmap font covering ASCII characters 32-127 (space through delete).

### Custom BMP Fonts

Custom fonts can be loaded from BMP files, which allows for:

- Different character sizes
- Extended character sets
- Decorative fonts for UI elements

The BMP font loader extracts individual character bitmaps from a BMP image based on a grid layout.

## ANSI Escape Sequences

konstruct supports a subset of ANSI escape sequences for terminal control:

| Sequence | Description |
|----------|-------------|
| `\033[H` | Move cursor to home position (0,0) |
| `\033[nA` | Move cursor up n lines |
| `\033[nB` | Move cursor down n lines |
| `\033[nC` | Move cursor right n characters |
| `\033[nD` | Move cursor left n characters |
| `\033[n;mH` | Move cursor to position (n,m) |
| `\033[2J` | Clear entire screen |
| `\033[K` | Clear line from cursor to end |
| `\033[0m` | Reset all attributes |
| `\033[30-37m` | Set foreground color |
| `\033[40-47m` | Set background color |

## Terminal Emulation

The enhanced terminal subsystem provides:

- Multiple virtual terminals/panes
- Theming support
- Command history
- Basic line editing
- Status line

Available terminal themes include:

- Default (blue background, white text)
- Dark (black background, light text)
- Light (light background, dark text)
- Blue (blue gradients)
- Green (green/black retro look)
- Retro (classic green on black)

## Code Examples

### Setting a Pixel in Graphics Mode

```c
// Set a pixel at (x,y) with color
void set_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < get_screen_width() && 
        y >= 0 && y < get_screen_height()) {
        uint8_t* framebuffer = (uint8_t*)0xA0000;
        framebuffer[y * get_screen_width() + x] = color;
    }
}
```

### Drawing Text with Custom Font

```c
// Draw text using a custom font
void draw_text(int x, int y, const char* text) {
    const Font* font = font_get_default();
    int current_x = x;
    
    while (*text) {
        // Draw character and advance cursor
        int width = font_draw_char(font, current_x, y, *text, 
                                  VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        current_x += width + font->spacing;
        text++;
    }
}
```

### Switching to Graphics Mode

```c
// Switch to 320x200 graphics mode
void switch_to_graphics() {
    // Update global variables
    set_is_graphics_mode(1);
    set_screen_width(320);
    set_screen_height(200);
    
    // Set the mode using BIOS
    if (vga_set_mode(VGA_MODE_320x200)) {
        // Draw header/UI elements
        vga_draw_filled_rect(0, 0, 320, 16, VGA_COLOR_DARK_GRAY);
        font_draw_string(font_get_default(), 4, 4, 
                       "Graphics Mode 320x200", 
                       VGA_COLOR_WHITE, VGA_COLOR_DARK_GRAY);
    }
}
```

### Using ANSI Escape Sequences

```c
// Print colored text using ANSI escape sequences
void print_colored_text() {
    printf("\033[31mThis text is red\033[0m\n");
    printf("\033[32mThis text is green\033[0m\n");
    printf("\033[44;37mWhite text on blue background\033[0m\n");
}
```
