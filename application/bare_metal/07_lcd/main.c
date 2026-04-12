/**
 * @file main.c
 * @brief Professional LCD Graphics Demo for JZ2440
 * 
 * Demonstrates LCD initialization, clearing, and primitive drawing (rectangles).
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_sdram.h"
#include "hal/hal_lcd.h"
#include "hal/hal_gpio.h"
#include <stdint.h>

/* --- Configuration --- */
#define HEARTBEAT_LED   GPF4

/**
 * @brief Simple software delay
 */
static void delay(volatile int count) {
    while (count--) {
        __asm__("nop");
    }
}

/**
 * @brief Draw a test pattern on the LCD
 */
static void draw_test_pattern(void) {
    hal_uart_puts("[LCD] Drawing test pattern...\r\n");

    /* 1. Clear screen with a professional background color */
    hal_lcd_clear(COLOR_BLACK);
    
    /* 2. Draw some color blocks */
    hal_lcd_draw_rect(50, 50, 100, 100, COLOR_RED);
    hal_lcd_draw_rect(200, 50, 100, 100, COLOR_GREEN);
    hal_lcd_draw_rect(350, 50, 80, 100, COLOR_YELLOW);
    
    /* 3. Draw a center crosshair */
    hal_lcd_draw_rect(0, LCD_HEIGHT/2 - 1, LCD_WIDTH, 2, COLOR_WHITE);
    hal_lcd_draw_rect(LCD_WIDTH/2 - 1, 0, 2, LCD_HEIGHT, COLOR_WHITE);

    hal_uart_puts("[LCD] Pattern complete.\r\n");
}

int main(void) {
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("      JZ2440 LCD DIAGNOSTIC TOOL        \r\n");
    hal_uart_puts("========================================\r\n");

    hal_uart_puts("[LCD] Initializing controller... ");
    hal_lcd_init();
    hal_uart_puts("DONE\r\n");

    /* 2. Heartbeat LED setup */
    hal_gpio_init_output(HEARTBEAT_LED);

    /* 3. Graphics Output */
    draw_test_pattern();

    hal_uart_puts("[SYS] Entering heartbeat loop.\r\n");

    /* 4. Main Loop: Heartbeat */
    while (1) {
        hal_gpio_toggle(HEARTBEAT_LED);
        delay(500000);
    }

    return 0;
}
