/**
 * @file main.c
 * @brief UART Console Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_uart.h"
#include "hal/hal_gpio.h"
#include <stdint.h>

extern void hal_system_init(void);

#define HEARTBEAT_LED   BSP_LED1

static void uart_echo_loop(void) {
    while (1) {
        char c = hal_uart_getc();
        hal_gpio_toggle(HEARTBEAT_LED);

        if (c == '\r') {
            hal_uart_puts("\r\n> ");
        } else if (c != '\n') {
            hal_uart_putc(c);
        }
    }
}

int main(void) {
    /* 1. Low-level Hardware (Clock) */
    bsp_clock_init();

    /* 2. Data Relocation */
    hal_system_init();

    /* 3. Board-level init (UART + GPIO heartbeat LED) */
    bsp_uart_init();
    bsp_gpio_init();

    BSP_PRINT_BANNER("03 UART Console Demo");
    hal_uart_puts("Settings: 115200 8N1\r\n");
    hal_uart_puts("Status  : Ready for input...\r\n");
    hal_uart_puts("----------------------------------------\r\n> ");

    uart_echo_loop();

    return 0;
}
