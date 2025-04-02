#ifndef TERMINAL_H
#define TERMINAL_H

#include "../libc/stdint.h"
#include "../libc/stddef.h"
#include "vga.h"

// Terminal color theme
typedef struct {
    uint8_t background;     // Main background color
    uint8_t foreground;     // Main text color
    uint8_t header_bg;      // Header background
    uint8_t header_fg;      // Header text
    uint8_t prompt_bg;      // Command prompt background
    uint8_t prompt_fg;      // Command prompt text
    uint8_t command_fg;     // User input color
    uint8_t output_fg;      // Command output color
    uint8_t error_fg;       // Error message color
    uint8_t status_fg;      // Status message color
    uint8_t border;         // Border/separator color
} terminal_theme_t;

// Terminal buffer structure
typedef struct {
    char* content;       // Content of the buffer
    int width;           // Width of the buffer (in characters)
    int height;          // Height of the buffer (in characters)
    int cursor_x;        // Cursor X position in the buffer
    int cursor_y;        // Cursor Y position in the buffer
    int scroll_offset;   // Scroll offset (top line of the visible area)
} terminal_buffer_t;

// Terminal pane structure
typedef struct {
    terminal_buffer_t* buffer; // Associated buffer
    int x;                     // X position of the pane (in characters)
    int y;                     // Y position of the pane (in characters)
    int width;                 // Width of the pane (in characters)
    int height;                // Height of the pane (in characters)
} terminal_pane_t;

// Redesigned terminal state
typedef struct {
    terminal_pane_t header;    // Header pane
    terminal_pane_t footer;    // Footer pane
    terminal_pane_t content;   // Main content pane
    terminal_theme_t theme;    // Color theme
    terminal_pane_t* active_pane; // Currently active pane
} terminal_t;

// Global terminal instance
extern terminal_t terminal;

// Terminal initialization and cleanup
void terminal_init(void);
void terminal_cleanup(void);

// Terminal pane management
void terminal_draw_pane(terminal_pane_t* pane);
void terminal_scroll_pane(terminal_pane_t* pane, int lines);
void terminal_set_active_pane(terminal_pane_t* pane);
void terminal_clear_content(void);
// Terminal output functions
void terminal_putchar(char c);

/**
 * Print a string to the terminal.
 * Supports ANSI escape sequences for formatting and cursor control.
 * 
 * @param str The string to print.
 */
void terminal_print(const char* str);

void terminal_println(const char* str);
void terminal_printf(const char* format, ...);

// Styled terminal output
void terminal_print_prompt(void);
void terminal_print_error(const char* message);
void terminal_print_status(const char* message);
void terminal_print_command(const char* command);
void terminal_print_separator(void);

// Terminal input functions
char terminal_getchar(void);
void terminal_readline(char* buffer, size_t size);

// Theme management
void terminal_set_theme(const terminal_theme_t* theme);
void terminal_load_theme(const char* theme_name);
void terminal_get_default_theme(terminal_theme_t* theme);

// // Predefined themes
// extern const terminal_theme_t THEME_DEFAULT;
// extern const terminal_theme_t THEME_DARK;
// extern const terminal_theme_t THEME_LIGHT;
// extern const terminal_theme_t THEME_BLUE;
// extern const terminal_theme_t THEME_GREEN;
// extern const terminal_theme_t THEME_RETRO;

#endif // TERMINAL_H
