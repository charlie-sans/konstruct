#ifndef GLOBALS_H
#define GLOBALS_H
#include "libc/libc.h"

// Global variables for screen mode and dimensions
extern int is_graphics_mode;  // 0 for text mode, 1 for graphics mode
extern int screen_width;
extern int screen_height;

// Filesystem mount status
int is_boot_device_mounted(void);
void set_boot_device_mounted(int status);

// Setter functions
void set_is_graphics_mode(int mode);
void set_screen_height(int height);
void set_screen_width(int width);

// Getter functions
int get_is_graphics_mode(void);
int get_screen_height(void);
int get_screen_width(void);

#endif /* GLOBALS_H */
