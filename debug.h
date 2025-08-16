#ifndef DEBUG_H
#define DEBUG_H

// Debug levels
#define DEBUG_ERROR   1
#define DEBUG_WARN    2
#define DEBUG_INFO    3
#define DEBUG_VERBOSE 4

// Current debug level (set to 3 for info level)
#define DEBUG_LEVEL DEBUG_INFO

// Debug print macro
#define debug_print(level, fmt, ...) \
    do { \
        if (level <= DEBUG_LEVEL) { \
            printf("[DEBUG:%d] " fmt, level, ##__VA_ARGS__); \
        } \
    } while (0)

// Panic function - halts the system with error message
void panic(const char* message);

// Debug breakpoint - triggers debugger if attached
void debug_break(void);

// Print keyboard state for debugging
void debug_keyboard_state(void);

#endif
