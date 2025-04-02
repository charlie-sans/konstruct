#include "globals.h"

// Global variables definitions
int is_graphics_mode = 0;  // 0 for text mode, 1 for graphics mode
int screen_width = 80;     // Default text mode width
int screen_height = 25;    // Default text mode height
int boot_device_mounted = 0; // 0 for not mounted, 1 for mounted

// Setter functions
void set_is_graphics_mode(int mode) {
    is_graphics_mode = mode;
}

void set_screen_height(int height) {
    screen_height = height;
}

void set_screen_width(int width) {
    screen_width = width;
}

void set_boot_device_mounted(int status) {
    boot_device_mounted = status;
}

// Getter functions
int get_is_graphics_mode(void) {
    return is_graphics_mode;
}

int get_screen_height(void) {
    return screen_height;
}

int get_screen_width(void) {
    return screen_width;
}

int is_boot_device_mounted(void) {
    return boot_device_mounted;
}
