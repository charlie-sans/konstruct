#include "libc.h"

// External functions from kernel.c
extern void print_char(char c);
extern char scancode_to_ascii(unsigned char scancode);
extern unsigned char read_scan_code(void);

// Helper function to handle ANSI escape codes
static void handle_ansi_escape(const char* seq) {
    // Parse the ANSI escape sequence and apply the corresponding action
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
    switch (cmd) {
        case 'm': // Set graphics mode (e.g., colors)
            for (int i = 0; i <= param_count; i++) {
                switch (params[i]) {
                    case 0: // Reset all attributes
                        // Reset colors to default
                        break;
                    case 30 ... 37: // Set foreground color
                        // Map params[i] to terminal foreground color
                        break;
                    case 40 ... 47: // Set background color
                        // Map params[i] to terminal background color
                        break;
                }
            }
            break;

        case 'H': // Cursor position (CSI row;colH or CSI row;colf)
        case 'f':
            if (param_count >= 1 && param_count >= 2) {
                // Move cursor to (params[0], params[1])
            }
            break;

        case 'J': // Clear screen (CSI nJ)
            if (params[0] == 2) {
                // Clear the entire screen
            }
            break;

        // Add more cases as needed for other ANSI commands
    }
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