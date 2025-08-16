#include "../libc.h"
#include "keyboard.h"

// PS/2 Keyboard controller ports
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_COMMAND_PORT 0x64

// PS/2 Controller commands
#define PS2_CMD_DISABLE_FIRST_PORT  0xAD
#define PS2_CMD_DISABLE_SECOND_PORT 0xA7
#define PS2_CMD_ENABLE_FIRST_PORT   0xAE
#define PS2_CMD_ENABLE_SECOND_PORT  0xA8
#define PS2_CMD_READ_CONFIG         0x20
#define PS2_CMD_WRITE_CONFIG        0x60
#define PS2_CMD_TEST_CONTROLLER     0xAA
#define PS2_CMD_TEST_FIRST_PORT     0xAB

// PS/2 Status register bits
#define PS2_STATUS_OUTPUT_FULL      0x01
#define PS2_STATUS_INPUT_FULL       0x02

// External functions
extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char data);

static int keyboard_initialized = 0;

// Wait for the PS/2 controller to be ready to accept commands
static void ps2_wait_write(void) {
    int timeout = 10000;
    while ((inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL) && timeout > 0) {
        timeout--;
    }
}

// Wait for the PS/2 controller to have data available
static void ps2_wait_read(void) {
    int timeout = 10000;
    while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) && timeout > 0) {
        timeout--;
    }
}

// Send a command to the PS/2 controller
static void ps2_send_command(unsigned char command) {
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, command);
}

// Send data to the PS/2 controller
static void ps2_send_data(unsigned char data) {
    ps2_wait_write();
    outb(PS2_DATA_PORT, data);
}

// Read data from the PS/2 controller
static unsigned char ps2_read_data(void) {
    ps2_wait_read();
    return inb(PS2_DATA_PORT);
}

// Initialize the PS/2 keyboard controller
int keyboard_init(void) {
    printf("Initializing PS/2 keyboard controller...\n");
    
    // Disable both PS/2 ports
    ps2_send_command(PS2_CMD_DISABLE_FIRST_PORT);
    ps2_send_command(PS2_CMD_DISABLE_SECOND_PORT);
    
    // Flush the output buffer
    inb(PS2_DATA_PORT);
    
    // Read the current configuration
    ps2_send_command(PS2_CMD_READ_CONFIG);
    unsigned char config = ps2_read_data();
    
    // Disable IRQs and translation
    config &= ~0x43; // Clear bits 0, 1, and 6
    
    // Write back the configuration
    ps2_send_command(PS2_CMD_WRITE_CONFIG);
    ps2_send_data(config);
    
    // Test the PS/2 controller
    ps2_send_command(PS2_CMD_TEST_CONTROLLER);
    unsigned char test_result = ps2_read_data();
    if (test_result != 0x55) {
        printf("PS/2 controller test failed: 0x%02X\n", test_result);
        return -1;
    }
    
    // Test the first PS/2 port (keyboard)
    ps2_send_command(PS2_CMD_TEST_FIRST_PORT);
    test_result = ps2_read_data();
    if (test_result != 0x00) {
        printf("PS/2 first port test failed: 0x%02X\n", test_result);
        return -1;
    }
    
    // Enable the first PS/2 port
    ps2_send_command(PS2_CMD_ENABLE_FIRST_PORT);
    
    // Reset the keyboard
    ps2_send_data(0xFF); // Reset command
    unsigned char ack = ps2_read_data();
    if (ack != 0xFA) {
        printf("Keyboard reset failed: 0x%02X\n", ack);
        return -1;
    }
    
    // Wait for the keyboard to complete reset
    unsigned char reset_result = ps2_read_data();
    if (reset_result != 0xAA) {
        printf("Keyboard reset completion failed: 0x%02X\n", reset_result);
        return -1;
    }
    
    // Enable keyboard scanning
    ps2_send_data(0xF4); // Enable scanning command
    ack = ps2_read_data();
    if (ack != 0xFA) {
        printf("Keyboard enable scanning failed: 0x%02X\n", ack);
        return -1;
    }
    
    keyboard_initialized = 1;
    printf("PS/2 keyboard controller initialized successfully\n");
    return 0;
}

// Check if keyboard data is available
int keyboard_data_available(void) {
    return (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) != 0;
}

// Read a scan code from the keyboard (non-blocking)
unsigned char keyboard_read_scancode(void) {
    if (!keyboard_data_available()) {
        return 0; // No data available
    }
    return inb(PS2_DATA_PORT);
}

// Read a scan code from the keyboard (blocking with timeout)
unsigned char keyboard_read_scancode_blocking(void) {
    int timeout = 1000000; // Large timeout for blocking read
    
    while (!keyboard_data_available() && timeout > 0) {
        timeout--;
        // Small delay
        for (volatile int i = 0; i < 100; i++);
    }
    
    if (timeout <= 0) {
        return 0; // Timeout
    }
    
    return inb(PS2_DATA_PORT);
}

// Check if keyboard is initialized
int keyboard_is_initialized(void) {
    return keyboard_initialized;
}
