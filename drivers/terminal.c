#include "terminal.h"
#include "../libc/libc.h"
#include "../fs/fs.h"

// External functions from kernel.c
extern void update_cursor(int x, int y);

// Global terminal instance
terminal_t terminal;

// Predefined themes
const terminal_theme_t THEME_DEFAULT = {
    VGA_COLOR_BLUE,        // background
    VGA_COLOR_WHITE,       // foreground
    VGA_COLOR_DARK_GRAY,   // header_bg
    VGA_COLOR_WHITE,       // header_fg
    VGA_COLOR_DARK_GRAY,   // prompt_bg
    VGA_COLOR_LIGHT_GREEN, // prompt_fg
    VGA_COLOR_LIGHT_CYAN,  // command_fg
    VGA_COLOR_WHITE,       // output_fg
    VGA_COLOR_LIGHT_RED,   // error_fg
    VGA_COLOR_YELLOW,      // status_fg
    VGA_COLOR_LIGHT_GRAY   // border
};

// Initialize the terminal
void terminal_init(void) {
    // Set default theme
    terminal_set_theme(&THEME_DEFAULT);

    // Clear the screen
    printf("\033[2J"); // ANSI escape code to clear the screen

    // Move cursor to the top-left corner
    printf("\033[H");

    // Print the header and footer
    terminal_draw_header();
    terminal_draw_footer();
}

// Draw the terminal header
void terminal_draw_header(void) {
    printf("\033[1;1H"); // Move cursor to the top-left corner
    printf("\033[48;5;%dm\033[38;5;%dm", terminal.theme.header_bg, terminal.theme.header_fg); // Set header colors
    printf(" MyOS Terminal - Welcome! "); // Example header text
    printf("\033[0m"); // Reset colors
}

// Draw the terminal footer
void terminal_draw_footer(void) {
    printf("\033[25;1H"); // Move cursor to the bottom-left corner
    printf("\033[48;5;%dm\033[38;5;%dm", terminal.theme.header_bg, terminal.theme.header_fg); // Set footer colors
    printf(" [F1] Help | [F2] Menu | [F3] Theme | [F4] Clear "); // Example footer text
    printf("\033[0m"); // Reset colors
}

// Clear the terminal content area
void terminal_clear_content(void) {
    printf("\033[2J"); // Clear the screen
    terminal_draw_header();
    terminal_draw_footer();
    printf("\033[2;1H"); // Move cursor to the content area
}

// Enhanced ANSI escape code parser
static void terminal_handle_ansi(const char* seq) {
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
        case 'H': // Cursor position (CSI row;colH or CSI row;colf)
        case 'f':
            if (param_count >= 1) terminal.active_pane->buffer->cursor_y = params[0] - 1;
            if (param_count >= 2) terminal.active_pane->buffer->cursor_x = params[1] - 1;
            update_cursor(terminal.active_pane->buffer->cursor_x, terminal.active_pane->buffer->cursor_y);
            break;

        case 'J': // Clear screen (CSI nJ)
            if (params[0] == 2) terminal_clear_content(); // Clear entire screen
            break;

        case 'm': // Set graphics mode (CSI n;m;m...m)
            for (int i = 0; i <= param_count; i++) {
                switch (params[i]) {
                    case 0: // Reset all attributes
                        terminal.theme.foreground = THEME_DEFAULT.foreground;
                        terminal.theme.background = THEME_DEFAULT.background;
                        break;
                    case 30 ... 37: // Set foreground color
                        terminal.theme.foreground = params[i] - 30;
                        break;
                    case 40 ... 47: // Set background color
                        terminal.theme.background = params[i] - 40;
                        break;
                }
            }
            break;

        // Add more cases as needed for other ANSI commands
    }
}

// Updated terminal_print function to handle ANSI sequences
void terminal_print(const char* str) {
    if (!str) return;

    while (*str) {
        if (*str == '\033' && *(str + 1) == '[') { // Detect ANSI escape sequence
            const char* seq_start = str + 2;
            const char* seq_end = seq_start;

            // Find the end of the escape sequence
            while (*seq_end && (*seq_end < '@' || *seq_end > '~')) {
                seq_end++;
            }

            if (*seq_end) {
                char seq[32] = {0};
                size_t seq_len = seq_end - seq_start + 1;
                if (seq_len < sizeof(seq)) {
                    strncpy(seq, seq_start, seq_len);
                    terminal_handle_ansi(seq);
                }
                str = seq_end + 1; // Skip the entire escape sequence
                continue;
            }
        }

        terminal_putchar(*str++);
    }
}

// Print a single character to the terminal
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal.active_pane->buffer->cursor_y++;
        terminal.active_pane->buffer->cursor_x = 0;
    } else if (c == '\r') {
        terminal.active_pane->buffer->cursor_x = 0;
    } else if (c == '\b') {
        if (terminal.active_pane->buffer->cursor_x > 0) {
            terminal.active_pane->buffer->cursor_x--;
            // Handle backspace character
            printf("\b \b"); // Move cursor back, print space, move back again
        }
    } else {
        // Print the character at the current cursor position
        putchar(c);
        terminal.active_pane->buffer->cursor_x++;
    }

    update_cursor(terminal.active_pane->buffer->cursor_x, terminal.active_pane->buffer->cursor_y);
}

// Print a string with a newline
void terminal_println(const char* str) {
    printf("%s\n", str);
}

// Print formatted text
void terminal_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Print the command prompt
void terminal_print_prompt(void) {
    printf("\033[38;5;%dm", terminal.theme.prompt_fg); // Set prompt color
    printf("user@myos:/$ "); // Example prompt
    printf("\033[0m"); // Reset colors
}

// Set the terminal theme
void terminal_set_theme(const terminal_theme_t* theme) {
    if (!theme) return;
    terminal.theme = *theme;
    terminal_clear_content(); // Refresh the terminal with the new theme
}

// Refresh the terminal display
void terminal_refresh(void) {
    terminal_clear_content();
}

// Load a named theme
void terminal_load_theme(const char* theme_name) {
    if (!theme_name) return;

    // if (strcmp(theme_name, "default") == 0) {
    //     terminal_set_theme(&THEME_DEFAULT);
    // } else if (strcmp(theme_name, "dark") == 0) {
    //     terminal_set_theme(&THEME_DARK);
    // } else if (strcmp(theme_name, "light") == 0) {
    //     terminal_set_theme(&THEME_LIGHT);
    // } else if (strcmp(theme_name, "blue") == 0) {
    //     terminal_set_theme(&THEME_BLUE);
    // } else if (strcmp(theme_name, "green") == 0) {
    //     terminal_set_theme(&THEME_GREEN);
    // } else if (strcmp(theme_name, "retro") == 0) {
    //     terminal_set_theme(&THEME_RETRO);
    // } else {
    //     // Unknown theme, use default
    //     terminal_set_theme(&THEME_DEFAULT);
    // }
}

// Print a status message
void terminal_print_status(const char* message) {
    printf("\033[38;5;%dm", terminal.theme.status_fg); // Set status color
    printf("%s\n", message);
    printf("\033[0m"); // Reset colors
}

// Read a line of input from the terminal
void terminal_readline(char* buffer, size_t size) {
    if (!buffer || size == 0) return;

    size_t pos = 0;
    while (pos < size - 1) {
        char c = getchar(); // Use standard input function
        if (c == '\n' || c == '\r') {
            buffer[pos] = '\0';
            printf("\n"); // Echo newline
            return;
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
                printf("\b \b"); // Handle backspace
            }
        } else {
            buffer[pos++] = c;
            putchar(c); // Echo character
        }
    }

    buffer[pos] = '\0'; // Null-terminate the string
}

// Ensure vprintf is linked
int vprintf(const char* format, va_list args) {
    return vsnprintf(NULL, 0, format, args); // Use vsnprintf as a fallback
}
