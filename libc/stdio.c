#include <libc.h>
#include <stddef.h>

#include <globals.h>
#include "drivers/serial.h"
#define screen_width 80
#define screen_height 25

char scancode_to_ascii(unsigned char scancode) {
    static int shift_pressed = 0;
    static int caps_lock_enabled = 0;

    // Handle key releases first (scancodes with high bit set)
    if (scancode & 0x80) {
        // Key release - high bit set
        unsigned char released_key = scancode & 0x7F; // Clear the high bit to get the key code
        
        // Handle shift key release
        if (released_key == 0x2A || released_key == 0x36) {
            shift_pressed = 0;
        }
        
        return 0; // No character output for key releases
    }

    // Handle key presses (scancodes without high bit)
    // Special keys
    if (scancode == 0x2A || scancode == 0x36) { // Shift key press
        shift_pressed = 1;
        return 0;
    } else if (scancode == 0x3A) { // Caps Lock key press
        caps_lock_enabled = !caps_lock_enabled;
        return 0;
    } else if (scancode == 0xE0) { // Extended key prefix
        return 0;
    } else if (scancode == 0x1D) { // Control key press
        return 0;
    }

    // Handle standard keys
    const char scancode_to_ascii_map_lower[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };

    const char scancode_to_ascii_map_upper[] = {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0,
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
        'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
    };
    
    if (scancode == 0x1C) { // Enter key
        return '\n';
    } else if (scancode == 0x0E) { // Backspace key
        return '\b';
    } else if (scancode < sizeof(scancode_to_ascii_map_lower)) {
        // Determine if we need upper or lower case
        if ((shift_pressed && !caps_lock_enabled) || (!shift_pressed && caps_lock_enabled)) {
            return scancode_to_ascii_map_upper[scancode];
        } else {
            return scancode_to_ascii_map_lower[scancode];
        }
    }
    
    return 0;  // Not a printable character
}

extern unsigned char read_scan_code(void);

// Add these globals for double buffering
static char text_buffer[screen_height][screen_width][2]; // [row][col][char/attribute]
static char* video_memory = (char*)VIDEO_MEMORY;
static int buffer_initialized = 0;
static int needs_update = 0;

// Initialize the text buffer
static void init_text_buffer() {
    if (buffer_initialized) return;
    
    // Clear the buffer
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            text_buffer[y][x][0] = ' ';
            text_buffer[y][x][1] = WHITE_ON_BLACK;
        }
    }
    
    // Also clear the actual screen
    for (int i = 0; i < screen_width * screen_height * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = WHITE_ON_BLACK;
    }
    
    buffer_initialized = 1;
}

// Flush the buffer to screen - only copy what has changed
static void flush_buffer() {
    if (!needs_update) return;
    
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            int offset = (y * screen_width + x) * 2;
            
            // Only update if the character or attribute has changed
            if (video_memory[offset] != text_buffer[y][x][0] || 
                video_memory[offset + 1] != text_buffer[y][x][1]) {
                
                video_memory[offset] = text_buffer[y][x][0];
                video_memory[offset + 1] = text_buffer[y][x][1];
            }
        }
    }
    
    needs_update = 0;
}

// Function to print a single character
void print_char(char c) {
    static int cursor_x = 0, cursor_y = 0;
    static int initialized = 0;
    static int serial_initialized = 0;
    
    // Initialize serial port if needed
    if (!serial_initialized) {
        serial_init(SERIAL_COM1_BASE);
        serial_initialized = 1;
    }

    // Send character to serial port
    serial_write_char(SERIAL_COM1_BASE, c);
    
    // Initialize buffer if needed
    if (!initialized) {
        init_text_buffer();
        cursor_x = 0;
        cursor_y = 0;
        initialized = 1;
    }
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            text_buffer[cursor_y][cursor_x][0] = ' ';
            text_buffer[cursor_y][cursor_x][1] = WHITE_ON_BLACK;
            needs_update = 1;
        }
    } else if (c == '\0') {
        // Null character just updates the cursor
    } else {
        text_buffer[cursor_y][cursor_x][0] = c;
        text_buffer[cursor_y][cursor_x][1] = WHITE_ON_BLACK;
        cursor_x++;
        needs_update = 1;
        
        if (cursor_x >= screen_width) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= screen_height) {
        // Scroll the buffer up by one line
        for (int y = 0; y < screen_height - 1; y++) {
            for (int x = 0; x < screen_width; x++) {
                text_buffer[y][x][0] = text_buffer[y + 1][x][0];
                text_buffer[y][x][1] = text_buffer[y + 1][x][1];
            }
        }

        // Clear the last line
        for (int x = 0; x < screen_width; x++) {
            text_buffer[screen_height - 1][x][0] = ' ';
            text_buffer[screen_height - 1][x][1] = WHITE_ON_BLACK;
        }

        // Move cursor up one row
        cursor_y = screen_height - 1;
        needs_update = 1;
    }

    // Update the hardware cursor
    update_cursor(cursor_x, cursor_y);
    
    // Flush buffer to screen
    flush_buffer();
}
void kernel_putchar(char c) {
    print_char(c); // Use the existing print_char function
}

// Function to print a string
void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}
// Clear the screen - now using the buffer
void clear_screen(void) {
    // Clear the buffer
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            text_buffer[y][x][0] = ' ';
            text_buffer[y][x][1] = WHITE_ON_BLACK;
        }
    }
    
    needs_update = 1;
    
    // Reset cursor to 0,0
    update_cursor(0, 0);
    
    // Flush buffer to screen to ensure changes take effect immediately
    flush_buffer();
}

// Function to apply ANSI effects programmatically
static void apply_ansi_effect(char cmd, int* params, int param_count) {
    switch (cmd) {
        case 'm': // Set graphics mode (e.g., colors)
            for (int i = 0; i <= param_count; i++) {
                switch (params[i]) {
                    case 0: // Reset all attributes
                        // Reset colors to default
                        // Example: reset terminal state variables
                        break;
                    case 30 ... 37: // Set foreground color
                        // Apply foreground color change
                        break;
                    case 40 ... 47: // Set background color
                        // Apply background color change
                        break;
                }
            }
            break;

        case 'H': // Cursor position (CSI row;colH or CSI row;colf)
        case 'f':
            if (param_count >= 1 && param_count >= 2) {
                // Move cursor to (params[0], params[1])
                // Example: update cursor position in terminal state
            }
            break;

        case 'J': // Clear screen (CSI nJ)
            if (params[0] == 2) {
                // Clear the entire screen
                // Example: reset terminal buffer
            }
            break;

        // Add more cases as needed for other ANSI commands
        default:
            break;
    }
}

// Refactored helper function to handle ANSI escape codes
static void handle_ansi_escape(const char* seq) {
    int params[8] = {0}; // Support up to 8 parameters
    int param_count = 0;
    const char* p = seq;

    // Parse parameters
    while (*p && param_count < 8) {
        if (*p >= '0' && *p <= '9') {
            params[param_count] = params[param_count] * 10 + (*p - '0');
        } else if (*p == ';') {
            param_count++;
        } else {
            break;
        }
        p++;
    }

    // Handle the final command character
    char cmd = *p;
    apply_ansi_effect(cmd, params, param_count);
}

// Enhanced putchar to handle ANSI escape sequences
int putchar(int c) {
    static int in_escape = 0;
    static char escape_seq[32];
    static int escape_len = 0;

    if (in_escape) {
        if (c >= '@' && c <= '~') { // End of escape sequence
            escape_seq[escape_len++] = c;
            escape_seq[escape_len] = '\0';
            handle_ansi_escape(escape_seq);
            in_escape = 0;
            escape_len = 0;
        } else if (escape_len < sizeof(escape_seq) - 1) {
            escape_seq[escape_len++] = c;
        }
        return c;
    }

    if (c == '\033') { // Start of escape sequence
        in_escape = 1;
        escape_len = 0;
        return c;
    }

    // Regular character output
    print_char((char)c); // Use the existing low-level function
    return c;
}


int puts(const char* s) {
    while (*s) {
        putchar(*s++);
    }
    putchar('\n');
    return 0;
}

int vsnprintf(const char* format, va_list args) {
    int printed = 0;
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 10);
                    char* s = buf;
                    while (*s) {
                        putchar(*s++);
                        printed++;
                    }
                    break;
                }
                case 'x': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 16);
                    char* s = buf;
                    while (*s) {
                        putchar(*s++);
                        printed++;
                    }
                    break;
                }
                case 's': {
                    char* s = va_arg(args, char*);
                    while (*s) {
                        putchar(*s++);
                        printed++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    printed++;
                    break;
                }
                case '%': {
                    putchar('%');
                    printed++;
                    break;
                }
                default:
                    putchar('%');
                    putchar(*format);
                    printed += 2;
                    break;
            }
        } else {
            putchar(*format);
            printed++;
        }
        format++;
    }
    
    return printed;
}

// Enhanced printf to handle ANSI escape codes
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int printed = 0;
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 10);
                    char* s = buf;
                    while (*s) {
                        putchar(*s++);
                        printed++;
                    }
                    break;
                }
                case 'x': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 16);
                    char* s = buf;
                    while (*s) {
                        putchar(*s++);
                        printed++;
                    }
                    break;
                }
                case 's': {
                    char* s = va_arg(args, char*);
                    while (*s) {
                        putchar(*s++);
                        printed++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    printed++;
                    break;
                }
                case '%': {
                    putchar('%');
                    printed++;
                    break;
                }
                default:
                    putchar('%');
                    putchar(*format);
                    printed += 2;
                    break;
            }
        } else {
            putchar(*format);
            printed++;
        }
        format++;
    }
    
    va_end(args);
    return printed;
}

char getchar(void) {
    char c;
    unsigned char scancode;

    // Keep reading scancodes until we get a valid character
    while (1) {
        // Wait for a scan code 
        scancode = read_scan_code();
        
        // Convert to ASCII and check if it's a printable character
        c = scancode_to_ascii(scancode);
        
        // If we got a printable character, return it
        if (c != 0) {
            return c;
        }
        
        // Otherwise, keep reading scancodes
    }
}

char* gets(char* str) {
    char* original_str = str;
    char c;
    
    while (1) {
        c = getchar();
        
        // Handle Enter key
        if (c == '\n' || c == '\r') {
            break;
        }
        
        if (c == '\b') {
            // Handle backspace
            if (str > original_str) {
                str--;
                // Erase the character on screen
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        } else {
            *str++ = c;
        }
    }
    
    *str = '\0';
    putchar('\n');
    
    return original_str;
}

// Convert string to integer
int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Process each digit
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

// Convert integer to string
char* itoa(int value, char* str, int base) {
    char* original_str = str;
    char* p = str;
    char tmp;
    int negative = 0;
    
    if (value < 0 && base == 10) {
        negative = 1;
        value = -value;
    }
    
    // Generate digits in reverse order
    do {
        int digit = value % base;
        *p++ = (digit < 10) ? '0' + digit : 'a' + digit - 10;
    } while ((value /= base) > 0);
    
    if (negative) {
        *p++ = '-';
    }
    
    *p = '\0';
    
    // Reverse the string
    p--;
    while (str < p) {
        tmp = *str;
        *str = *p;
        *p = tmp;
        str++;
        p--;
    }
    
    return original_str;
}

//sprintf implementation
int sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int printed = 0;
    char* original_str = str;
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 10);
                    char* s = buf;
                    while (*s) {
                        *str++ = *s++;
                        printed++;
                    }
                    break;
                }
                case 'x': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 16);
                    char* s = buf;
                    while (*s) {
                        *str++ = *s++;
                        printed++;
                    }
                    break;
                }
                case 's': {
                    char* s = va_arg(args, char*);
                    while (*s) {
                        *str++ = *s++;
                        printed++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *str++ = c;
                    printed++;
                    break;
                }
                case '%': {
                    *str++ = '%';
                    printed++;
                    break;
                }
                default:
                    *str++ = '%';
                    *str++ = *format;
                    printed += 2;
                    break;  
            }
        } else {
            *str++ = *format;
            printed++;
        }
        format++;
    }
    *str = '\0';  // Null-terminate the string

    va_end(args);
    return printed;
}

//snprintf implementation
int snprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int printed = 0;
    char* original_str = str;
    
    while (*format && printed < size - 1) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 10);
                    char* s = buf;
                    while (*s && printed < size - 1) {
                        *str++ = *s++;
                        printed++;
                    }
                    break;
                }
                case 'x': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 16);
                    char* s = buf;
                    while (*s && printed < size - 1) {
                        *str++ = *s++;
                        printed++;
                    }
                    break;
                }
                case 's': {
                    char* s = va_arg(args, char*);
                    while (*s && printed < size - 1) {
                        *str++ = *s++;
                        printed++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    if (printed < size - 1) {
                        *str++ = c;
                        printed++;
                    }
                    break;
                }
                case '%': {
                    if (printed < size - 1) {
                        *str++ = '%';
                        printed++;
                    }
                    break;
                }
                default:
                    if (printed < size - 1) {
                        *str++ = '%';
                        printed++;
                    }
                    if (printed < size - 1) {
                        *str++ = *format;
                        printed++;
                    }
                    break;  
            }
        } else {
            if (printed < size - 1) {
                *str++ = *format;
                printed++;
            }
        }
        format++;
    }
    *str = '\0';  // Null-terminate the string

    va_end(args);
    return printed;
}