/**
 * @file main.c
 * @brief LCD Graphics Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_delay.h"
#include "hal/hal_gpio.h"
#include "hal/hal_lcd.h"
#include "hal/hal_uart.h"
#include <stdint.h>

extern void hal_system_init(void);

#define HEARTBEAT_LED BSP_LED1

static void draw_demo_ui(void) {
    hal_uart_puts("[LCD] Drawing UI elements...\r\n");

    hal_lcd_clear(COLOR_BLACK);
    hal_lcd_draw_rect(50, 50, 100, 100, COLOR_RED);
    hal_lcd_draw_rect(200, 50, 100, 100, COLOR_GREEN);
    hal_lcd_draw_rect(350, 50, 80, 100, COLOR_BLUE);

    hal_uart_puts("[LCD] UI Drawing complete.\r\n");
}

int main(void) {
    hal_system_init();

    /* SDRAM 应用由 Bootloader 加载时，Clock/SDRAM 已初始化 */
    bsp_uart_init();
    bsp_lcd_init();
    bsp_gpio_init();

    BSP_PRINT_BANNER("13 LCD Graphics Demo");
    hal_uart_puts("Panel  : 480x272, 16 bpp RGB565\r\n");
    hal_uart_puts("FB Addr: 0x33800000\r\n");

    draw_demo_ui();

    hal_uart_puts("[System] Application is running.\r\n");
    while (1) {
        hal_gpio_toggle(HEARTBEAT_LED);
        hal_delay(500000);
    }

    return 0;
}
