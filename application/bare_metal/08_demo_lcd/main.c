/**
 * @file main.c
 * @brief Professional LCD Graphics Demo (Simplified Boot Pattern)
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_sdram.h"
#include "hal/hal_lcd.h"
#include "hal/hal_gpio.h"
#include <stdint.h>

void hal_system_init(void);

#define HEARTBEAT_LED   GPF4

int main(void) {
    /* 1. Low-level Hardware (LCD and SDRAM both need Clock and SDRAM init) */
    hal_clock_init();
    hal_sdram_init();

    /* 2. Data Relocation */
    hal_system_init();

    /* 3. Peripheral Initialization */
    hal_uart_init(115200);
    hal_lcd_init();
    hal_gpio_init_output(HEARTBEAT_LED);

    hal_uart_puts("\r\n--- JZ2440 LCD Demo ---\r\n");
    
    hal_lcd_clear(COLOR_BLACK);
    hal_lcd_draw_rect(50, 50, 100, 100, COLOR_RED);
    hal_lcd_draw_rect(200, 50, 100, 100, COLOR_GREEN);
    hal_lcd_draw_rect(350, 50, 80, 100, COLOR_YELLOW);

    while (1) {
        hal_gpio_toggle(HEARTBEAT_LED);
        for(volatile int d=0; d<500000; d++);
    }
    return 0;
}
