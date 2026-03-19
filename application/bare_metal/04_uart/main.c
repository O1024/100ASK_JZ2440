/**
 * @file main.c
 * @brief UART Echo Application for JZ2440
 * 
 * Professional version: Robust UART character handling and heartbeat feedback.
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_gpio.h"

/* --- Configuration --- */
#define UART_BAUD_RATE  115200
#define HEARTBEAT_LED   GPF4

/**
 * @brief Print a welcome banner via UART
 */
static void print_banner(void) {
    hal_uart_puts("\r\n");
    hal_uart_puts("****************************************\r\n");
    hal_uart_puts("*      JZ2440 Professional SDK         *\r\n");
    hal_uart_puts("*          UART Echo Demo              *\r\n");
    hal_uart_puts("****************************************\r\n");
    hal_uart_puts("Baud: 115200, Format: 8N1\r\n");
    hal_uart_puts("Ready! Type characters to echo...\r\n\r\n");
}

int main(void) {
    /* 1. Hardware Initialization */
    hal_clock_init();
    hal_uart_init(UART_BAUD_RATE);
    
    /* Heartbeat LED: Init as output and turn OFF */
    hal_gpio_init_output(HEARTBEAT_LED);
    hal_gpio_set(HEARTBEAT_LED, GPIO_HIGH);

    print_banner();

    /* 2. Main Loop: Interactive Echo */
    while (1) {
        /* Wait for and receive a single character */
        char c = hal_uart_getc();

        /* Visual feedback on LED whenever data is received */
        hal_gpio_toggle(HEARTBEAT_LED);

        /* Professional Echo Logic: Handle CR (\r) and LF (\n) */
        if (c == '\r' || c == '\n') {
            hal_uart_puts("\r\n");
        } else {
            /* Echo back the original character */
            hal_uart_putc(c);
        }
    }

    return 0;
}
