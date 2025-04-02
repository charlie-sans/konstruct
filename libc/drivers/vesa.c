#include "vga.h"
#include "bios.h"  // Include our BIOS functions header
#include "../libc/libc.h"
#include "serial.h"

// External functions from kernel.c
extern void outb(unsigned short port, unsigned char data);
extern unsigned char inb(unsigned short port);

// Function to call VESA BIOS functions using our BIOS interrupt handler
void call_vesa_bios(uint16_t ax, uint16_t es, uint16_t di, uint16_t* result) {
    // Create a regs16_t structure as defined in bios.h
    regs16_t regs = {0};
    
    // Set up input registers
    regs.ax = ax;
    // ES:DI is a real mode segmented address - convert linear address to segment:offset
    regs.es = es;
    regs.di = di;
    
    // Call BIOS interrupt 0x10 (video services)
    vga_bios_call(&regs);
    
    // Get output values
    if (result) {
        *result = regs.ax;
    }
}

// Get VBE controller information
int vga_get_vbe_controller_info(vbe_controller_info_t* info) {
    // Allocate a buffer in low memory for the controller info
    // In a real implementation, this would need to use a real-mode accessible buffer
    memset(info, 0, sizeof(vbe_controller_info_t));
    
    // Set "VBE2" signature to get VBE 2.0+ info
    info->signature[0] = 'V';
    info->signature[1] = 'B';
    info->signature[2] = 'E';
    info->signature[3] = '2';
    
    // Set up registers for INT 0x10, AX=0x4F00 (get VBE controller info)
    regs16_t regs = {0};
    regs.ax = 0x4F00;
    // ES:DI points to the buffer - setting ES:DI in real mode
    regs.es = (uint32_t)info >> 4;  // Segment 
    regs.di = (uint32_t)info & 0xF; // Offset
    
    // Call BIOS interrupt
    vga_bios_call(&regs);
    
    // Check if function was successful (AX=0x004F)
    if ((regs.ax & 0xFFFF) != 0x004F) {
        return 0; // VBE function failed
    }
    
    // Verify "VESA" signature
    if (info->signature[0] != 'V' || info->signature[1] != 'E' || 
        info->signature[2] != 'S' || info->signature[3] != 'A') {
        return 0; // Invalid signature
    }
    
    return 1; // Success
}

// Get VBE mode information
int vga_get_vbe_mode_info(uint16_t mode, vbe_mode_info_t* info) {
    // Clear the info structure
    memset(info, 0, sizeof(vbe_mode_info_t));
    
    // Set up registers for INT 0x10, AX=0x4F01 (get VBE mode info)
    regs16_t regs = {0};
    regs.ax = 0x4F01;
    regs.cx = mode;
    // ES:DI points to the buffer - setting ES:DI in real mode
    regs.es = (uint32_t)info >> 4;  // Segment
    regs.di = (uint32_t)info & 0xF; // Offset
    
    // Call BIOS interrupt
    vga_bios_call(&regs);
    
    // Check if function was successful (AX=0x004F)
    if ((regs.ax & 0xFFFF) != 0x004F) {
        return 0; // VBE function failed
    }
    
    return 1; // Success
}

// Set VBE mode
int vga_set_vbe_mode(uint16_t mode, vbe_mode_info_t* mode_info) {
    // First get mode information
    if (!vga_get_vbe_mode_info(mode, mode_info)) {
        serial_write_string(SERIAL_COM1_BASE, "Failed to get mode info for VBE mode\n");
        return 0;
    }
    
    // Check if mode is supported
    if (!(mode_info->attributes & 0x01)) {
        serial_write_string(SERIAL_COM1_BASE, "VBE mode is not supported\n");
        return 0;
    }
    
    // Check if linear framebuffer is available (bit 7 set in mode)
    uint16_t linear_mode = mode | 0x4000;
    
    // Set up registers for INT 0x10, AX=0x4F02 (set VBE mode)
    regs16_t regs = {0};
    regs.ax = 0x4F02;
    regs.bx = linear_mode;
    
    // Call BIOS interrupt
    vga_bios_call(&regs);
    
    // Check if function was successful (AX=0x004F)
    if ((regs.ax & 0xFFFF) != 0x004F) {
        serial_write_string(SERIAL_COM1_BASE, "VBE function failed\n");
        return 0;
    }
    
    // Update framebuffer information
    // In a real implementation, you might need to map the physical framebuffer address
    // to a virtual address that your kernel can access
    
    return 1; // Success
}

// Get list of available VBE modes
int vga_get_vbe_modes(uint16_t* modes, int max_modes) {
    vbe_controller_info_t controller_info;
    
    // Get controller info
    if (!vga_get_vbe_controller_info(&controller_info)) {
        return 0;
    }
    
    // For testing, return some common VBE modes
    if (max_modes >= 4) {
        modes[0] = VGA_MODE_640x480;
        modes[1] = VGA_MODE_800x600;
        modes[2] = VGA_MODE_1024x768;
        modes[3] = 0xFFFF; // Terminator
        return 4;
    } else {
        for (int i = 0; i < max_modes; i++) {
            modes[i] = i < 3 ? VGA_MODE_640x480 + (i * 2) : 0xFFFF;
        }
        return max_modes;
    }
}

// Draw a pixel to a VBE framebuffer
void vga_draw_pixel_vbe(int x, int y, uint32_t color) {
    // Only proceed if we're in graphics mode
    if (vga_screen.text_mode) {
        return;
    }
    
    // Bounds check
    if (x < 0 || x >= vga_screen.width || y < 0 || y >= vga_screen.height) {
        return;
    }
    
    // Handle different bit depths
    switch (vga_screen.bpp) {
        case 8: {
            // 8 bits per pixel (256 colors)
            uint8_t* fb = (uint8_t*)vga_screen.framebuffer;
            fb[y * vga_screen.width + x] = (uint8_t)color;
            break;
        }
        
        case 16: {
            // 16 bits per pixel (High color)
            uint16_t* fb = (uint16_t*)vga_screen.framebuffer;
            fb[y * vga_screen.width + x] = (uint16_t)color;
            break;
        }
        
        case 24: {
            // 24 bits per pixel (True color) - packed as 3 bytes
            uint8_t* fb = (uint8_t*)vga_screen.framebuffer;
            int offset = (y * vga_screen.width + x) * 3;
            fb[offset] = color & 0xFF;           // Blue
            fb[offset + 1] = (color >> 8) & 0xFF; // Green
            fb[offset + 2] = (color >> 16) & 0xFF; // Red
            break;
        }
        
        case 32: {
            // 32 bits per pixel (True color with alpha/reserved byte)
            uint32_t* fb = (uint32_t*)vga_screen.framebuffer;
            fb[y * vga_screen.width + x] = color;
            break;
        }
    }
}
