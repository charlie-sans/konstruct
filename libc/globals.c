#include "libc.h"

// Define the global variables declared as extern in libc.h
int screen_width = DEFAULT_WIDTH;
int screen_height = DEFAULT_HEIGHT;
int is_graphics_mode = 0;  // 0 for text mode, 1 for graphics mode
