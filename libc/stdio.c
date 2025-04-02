#include <libc.h>
#include <stddef.h>

#include <globals.h>
#define screen_width 80
#define screen_height 25

char scancode_to_ascii(unsigned char scancode) {
    static int shift_pressed = 0;
    static int caps_lock_enabled = 0;

    // Handle shift key press and release
    if (scancode == 0x2A || scancode == 0x36) { // Shift key press
        shift_pressed = 1;
        return 0;
    } else if (scancode == 0xAA || scancode == 0xB6) { // Shift key release
        shift_pressed = 0;
        return 0;
    }

    // Handle Caps Lock
    if (scancode == 0x3A) { // Caps Lock press
        caps_lock_enabled = !caps_lock_enabled;
       
    }

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
    } else if (scancode == 0x39) { // Space key
        return ' ';
    } else if (scancode == 0x3A) { // Caps Lock key
        return 0; // No character output for Caps Lock
    }
    else if (scancode == 0xE0) { // Extended key prefix
        return 0; // No character output for extended keys
    } else if (scancode == 0xE1) { // Pause/Break key
        return 0; // No character output for Pause/Break
    } else if (scancode == 0xE2) { // Print Screen key
        return 0; // No character output for Print Screen
    } else if (scancode == 0xE3) { // Insert key
        return 0; // No character output for Insert
    } else if (scancode == 0xE4) { // Home key
        return 0; // No character output for Home
    } else if (scancode == 0xE5) { // Page Up key
        return 0; // No character output for Page Up
    } else if (scancode == 0xE6) { // Delete key
        return 0; // No character output for Delete
    } else if (scancode == 0xE7) { // End key
        return 0; // No character output for End
    } else if (scancode == 0xE8) { // Page Down key
        return 0; // No character output for Page Down
    } else if (scancode == 0xE9) { // Arrow keys and other special keys can be handled here as needed.
        return 0; // Not a printable character
    }
    if (scancode < sizeof(scancode_to_ascii_map_lower)) {
        if ((shift_pressed && !caps_lock_enabled) || (!shift_pressed && caps_lock_enabled)) {
            return scancode_to_ascii_map_upper[scancode];
        } else {
            return scancode_to_ascii_map_lower[scancode];
        }
    }
    return 0;  // Not a printable character
}
extern unsigned char read_scan_code(void);

// Function to print a single character
void print_char(char c) {
    static int cursor_x = 0, cursor_y = 0;
    char* video_memory = (char*)VIDEO_MEMORY;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            int offset = (cursor_y * 80 + cursor_x) * 2;
            video_memory[offset] = ' ';
            video_memory[offset + 1] = WHITE_ON_BLACK;
        }
    } else {
        int offset = (cursor_y * 80 + cursor_x) * 2;
        video_memory[offset] = c;
        video_memory[offset + 1] = WHITE_ON_BLACK;
        cursor_x++;
        if (cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= screen_height) {
        // Scroll the screen up by one line
        for (int i = 0; i < (screen_height - 1) * 80 * 2; i++) {
            video_memory[i] = video_memory[i + 80  * 2];
        }

        // Clear the last line
        for (int i = (screen_height - 1) * 80 * 2; 
             i < screen_height * 80 * 2; i += 2) {
            video_memory[i] = ' ';
            video_memory[i + 1] = WHITE_ON_BLACK;
        }

        // Move cursor up one row
        cursor_y = screen_height - 1;
    }

    // Update the hardware cursor
    update_cursor(cursor_x, cursor_y);
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
    unsigned char scancode;

    // Wait for a valid character
    while (1) {
        scancode = read_scan_code();

        // Check if it's a key press (not a key release)
        if (!(scancode & 0x80)) {
            char c = scancode_to_ascii(scancode);
            if (c) {
                // Return the character without echoing it
                // (echoing will be handled by the caller)
                return c;
            }
        }
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