#include "libc.h"

// External functions from kernel.c
extern void print_char(char c);
extern char read_scan_code(void);
extern char scancode_to_ascii(char scancode);

int putchar(int c) {
    print_char((char)c);
    return c;
}

int puts(const char* s) {
    while (*s) {
        putchar(*s++);
    }
    putchar('\n');
    return 0;
}

// Very basic printf implementation
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
    char scancode;
    
    // Wait for a valid character
    while (1) {
        scancode = read_scan_code();
        
        // Check if it's a key press (not a key release)
        if (!(scancode & 0x80)) {
            // Special handling for the Enter key
            if (scancode == 0x1C) {  // 0x1C is the scan code for Enter
                putchar('\n');
                return '\n';
            }
            
            // Handle other printable characters
            char c = scancode_to_ascii(scancode);
            if (c) {
                putchar(c);
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
