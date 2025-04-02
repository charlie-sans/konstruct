#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>
#include <stddef.h>
#include "../font.h"        // For Font type
#include "fs/bootdev.h"     // For boot_device_type_t
#include "drivers/vga.h"    // For vga_screen_t

/* VGA constants */
#define VIDEO_MEMORY         0xB8000
#define DEFAULT_WIDTH        80
#define DEFAULT_HEIGHT       25
#define WHITE_ON_BLACK       0x07

/* VGA modes */
#define VGA_MODE_TEXT_80x25  0x03
#define VGA_MODE_320x200     0x13

/* Keyboard I/O ports */
#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64

/* VGA color constants */
#define VGA_COLOR_BLACK      0
#define VGA_COLOR_BLUE       1
#define VGA_COLOR_GREEN      2
#define VGA_COLOR_CYAN       3
#define VGA_COLOR_RED        4
#define VGA_COLOR_MAGENTA    5
#define VGA_COLOR_BROWN      6
#define VGA_COLOR_LIGHT_GRAY 7
#define VGA_COLOR_DARK_GRAY  8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED  12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW     14
#define VGA_COLOR_WHITE      15

/* Boot device types */
#define BOOT_DEV_UNKNOWN     0
#define BOOT_DEV_CDROM       1
#define BOOT_DEV_HDD         2
#define BOOT_DEV_USB         3

/* Filesystem constants */
#define FS_SUCCESS           0
#define FS_MAX_PATH_LENGTH   256

/* Screen handling functions */
void clear_screen(void);
void print_char(char c);
void update_cursor(int x, int y);
void kernel_putchar(char c);
void print_string(const char* str);

/* Port I/O functions */
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
unsigned char read_scan_code(void);

/* Screen mode functions */
int set_vbe_mode(int width, int height, int bpp);
int vga_set_mode(int mode);
//set_screen_width(int width);
void set_screen_height(int height);
void set_screen_width(int width);
int get_screen_height(void);
void set_is_graphics_mode(int mode);
/* System control functions */
void reboot(void);
void soft_reboot(void);

/* Command handling functions */
void handle_command(const char* cmd);
void handle_run_command(const char* cmd);

/* Terminal handling functions */
void terminal_clear_content(void);
void terminal_load_theme(const char* theme_name);
void terminal_print_status(const char* message);
void terminal_println(const char* message);
void vga_terminal_init(void);
void vga_terminal_write(const char* str);
void vga_terminal_set_color(uint8_t fg, uint8_t bg);

/* Console output function */
void console_write(const char* str);

// /* Graphics functions */
// void vga_draw_filled_rect(int x, int y, int width, int height, uint8_t color);
// int font_draw_string(const Font* font, int x, int y, const char* str, uint32_t color, uint32_t bgcolor);
// // const Font* font_get_default(void);

/* Filesystem functions */
int fs_listdir(const char* path, char* buffer, size_t buffer_size);
int fs_chdir(const char* path);
char* fs_getcwd(char* buffer, size_t buffer_size);
int fs_mkdir(const char* path);
int fs_create(const char* path);
int fs_delete(const char* path);
int fs_getsize(const char* path);
int fs_read(const char* path, void* buffer, size_t size, size_t offset);
int fs_write(const char* path, const void* buffer, size_t size, size_t offset);
int fs_exists(const char* path);
int fs_remount(const char* path);

/* Boot device functions */
boot_device_type_t bootdev_get_type(void);
const char* bootdev_get_type_name(void);

/* ELF execution */
int elf_execute(const char* filename, int argc, char** argv);

#endif /* _STDLIB_H */
