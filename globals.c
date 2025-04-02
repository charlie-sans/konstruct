#include "globals.h"
#include "libc/libc.h"  // For DEFAULT_WIDTH and DEFAULT_HEIGHT


// Getter functions that can be called from any file
int get_screen_width(void) {
    return screen_width;
}

int get_screen_height(void) {
    return screen_height;
}

int get_is_graphics_mode(void) {
    return is_graphics_mode;
}

// Setter functions
void set_screen_width(int width) {
    screen_width = width;
}

void set_screen_height(int height) {
    screen_height = height;
}

void set_is_graphics_mode(int mode) {
    is_graphics_mode = mode;
}
