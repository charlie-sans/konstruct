#ifndef GLOBALS_H
#define GLOBALS_H
#include "libc/libc.h"

void set_is_graphics_mode(int mode);
void set_screen_height(int height);
void set_screen_width(int width); // Setter functionsint get_is_graphics_mode(void);int get_screen_height(void);int get_screen_width(void);extern inline int is_graphics_mode = 0;  // 0 for text mode, 1 for graphics mode
#endif /* GLOBALS_H */
