#include "terminal.h"
#include <libc.h>
#include <fs/fs.h>

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

    // Set default color attribute


    // Clear the screen
    terminal_clear_content();

    // Move cursor to the top-left corner
    terminal.active_pane->buffer->cursor_x = 0;
    terminal.active_pane->buffer->cursor_y = 0;
    update_cursor(terminal.active_pane->buffer->cursor_x, terminal.active_pane->buffer->cursor_y);

    // Print the header and footer
    terminal_draw_header();
    terminal_draw_footer();
}

// Draw the terminal header
void terminal_draw_header(void) {
    // Implementation for drawing header (if applicable)
}

// Draw the terminal footer
void terminal_draw_footer(void) {
    // Implementation for drawing footer (if applicable)
}

// Clear the terminal content area
void terminal_clear_content(void) {
    char* video_memory = (char*)0xB8000;
    uint8_t attribute = (terminal.theme.background << 4) | (terminal.theme.foreground & 0x0F);

    // Clear the entire screen
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = attribute;
    }

    // Reset cursor position
    terminal.active_pane->buffer->cursor_x = 0;
    terminal.active_pane->buffer->cursor_y = 0;
    update_cursor(0, 0);
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
    // Calculate the offset into video memory
    int offset = (terminal.active_pane->buffer->cursor_y * 80 + terminal.active_pane->buffer->cursor_x) * 2;
    char* video_memory = (char*)0xB8000;
    uint8_t attribute = (terminal.theme.background << 4) | (terminal.theme.foreground & 0x0F);

    if (c == '\n') {
        terminal.active_pane->buffer->cursor_y++;
        terminal.active_pane->buffer->cursor_x = 0;
    } else if (c == '\r') {
        terminal.active_pane->buffer->cursor_x = 0;
    } else if (c == '\b') {
        if (terminal.active_pane->buffer->cursor_x > 0) {
            terminal.active_pane->buffer->cursor_x--;
            // Handle backspace character by writing a space
            video_memory[offset - 2] = ' ';
            video_memory[offset - 1] = attribute;
        }
    } else {
        // Print the character at the current cursor position
        video_memory[offset] = c;
        video_memory[offset + 1] = attribute;  // Fix: write attribute to the next byte
        terminal.active_pane->buffer->cursor_x++;
    }

    // Handle scrolling
    if (terminal.active_pane->buffer->cursor_x >= 80) {
        terminal.active_pane->buffer->cursor_x = 0;
        terminal.active_pane->buffer->cursor_y++;
    }

    if (terminal.active_pane->buffer->cursor_y >= 25) {
        // Scroll the screen up by one line
        for (int i = 0; i < 24 * 80 * 2; i++) {
            video_memory[i] = video_memory[i + 80 * 2];
        }

        // Clear the last line
        for (int i = 24 * 80 * 2; i < 25 * 80 * 2; i += 2) {  // Fix: correct loop bounds
            video_memory[i] = ' ';
            video_memory[i + 1] = attribute;  // Fix: set attribute byte
        }
        terminal.active_pane->buffer->cursor_y = 24;
    }

    // Update the hardware cursor with current position
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
    printf("user@Konstruct:/$ "); // Example prompt

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

