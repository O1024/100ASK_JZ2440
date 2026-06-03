/**
 * @file main.c
 * @brief Professional UART Console Application
 * 
 * This example demonstrates professional UART interaction:
 * 1. Configuration: 115200 baud, 8 data bits, No parity, 1 stop bit (8N1).
 * 2. Robust line-ending handling (mapping CR to CRLF).
 * 3. Separation of hardware setup from the interactive loop.
 * 4. Visual heartbeat indication during blocking polling.
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_gpio.h"
#include <stdint.h>

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

#define UART_BAUD_RATE  115200
#define HEARTBEAT_LED   GPF4

/**
 * @brief Initialize hardware peripherals for the console
 */
static void hw_init(void) {
    /* Initialize UART with standard 8N1 configuration */
    hal_uart_init(UART_BAUD_RATE);
    
    /* Initialize heartbeat LED (Active Low, default Off) */
    hal_gpio_init_output(HEARTBEAT_LED);
    hal_gpio_set(HEARTBEAT_LED, GPIO_HIGH);
}

/**
 * @brief Print the system welcome banner
 */
static void print_banner(void) {
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("    JZ2440 Professional UART Console    \r\n");
    hal_uart_puts("========================================\r\n");
    hal_uart_puts("System   : ARM920T (S3C2440)\r\n");
    hal_uart_puts("Settings : 115200 8N1\r\n");
    hal_uart_puts("Status   : Ready for input...\r\n");
    hal_uart_puts("----------------------------------------\r\n> ");
}

/**
 * @brief Main interactive echo loop
 * 
 * Polls the UART RX register. When a character is received, it toggles
 * the heartbeat LED and echoes the character back. It specifically
 * handles Carriage Return ('\r') by echoing a full CRLF sequence.
 */
static void uart_echo_loop(void) {
    while (1) {
        /* Blocking read: waits until a character is available */
        char c = hal_uart_getc();
        
        /* Visual feedback that data was received */
        hal_gpio_toggle(HEARTBEAT_LED);
        
        /* Terminal emulation: Handle enter key (CR) */
        if (c == '\r') {
            hal_uart_puts("\r\n> ");
        } 
        /* Ignore bare Newlines if terminal sends CRLF */
        else if (c != '\n') {
            hal_uart_putc(c);
        }
    }
}

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 1. Low-level Hardware (Clock) */
    hal_clock_init();

    /* 2. Data Relocation */
    hal_system_init();

    /* 4. Console Setup */
    hw_init();
    print_banner();

    /* 5. Enter Interactive Loop */
    uart_echo_loop();

    return 0;
}
