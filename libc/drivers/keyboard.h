#ifndef KEYBOARD_H
#define KEYBOARD_H

// Initialize the PS/2 keyboard controller
int keyboard_init(void);

// Check if keyboard data is available
int keyboard_data_available(void);

// Read a scan code from the keyboard (non-blocking)
unsigned char keyboard_read_scancode(void);

// Read a scan code from the keyboard (blocking with timeout)
unsigned char keyboard_read_scancode_blocking(void);

// Check if keyboard is initialized
int keyboard_is_initialized(void);

#endif // KEYBOARD_H
