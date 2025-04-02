#ifndef KERNEL_H
#define KERNEL_H
#include <font.h>
#include <globals.h>
#include <stddef.h>




// VGA/Display related functions
extern void vga_init(void);
extern void clear_screen(void);
extern void print_string(const char* str);
extern void print_char(char c);
extern void kernel_putchar(char c);
extern int set_vbe_mode(int width, int height, int bpp);
extern int vga_set_mode(int mode);


// Input related functions
extern unsigned char read_scan_code(void);
extern char scancode_to_ascii(unsigned char scancode);

// Port I/O functions
extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char data);

// Terminal functions
extern void terminal_println(const char* str);
extern void terminal_print_prompt(void);
extern void terminal_readline(char* buffer, size_t size);
extern void terminal_clear_content(void);

// Font related functions
extern void font_init(void);
extern void bmp_font_init(void);
extern void bmp_font_cleanup(void);
extern int font_draw_char(const  Font* font, int x, int y, char c, uint32_t color, uint32_t bgcolor);
extern int font_draw_string(const  Font* font, int x, int y, const char* str, uint32_t color, uint32_t bgcolor);

// File system functions
extern void fs_init(void);
extern void fs_cleanup(void);
extern int elf_execute(const char* filename, int argc, char** argv);

// Shell functions
extern void shell_main(void);
extern void handle_command(const char* cmd);
extern void handle_run_command(const char* cmd);

// System control functions
extern void reboot(void);
extern void soft_reboot(void);

// Memory management (used by libc)
extern void* _sbrk(int incr);

// Console output (used by libc)
extern int _write(int fd, const void* buf, size_t count);
extern int _read(int fd, void* buf, size_t count);

// Structure definitions needed for function declarations
struct Font;

#endif /* KERNEL_H */