/**
 * @file main.c
 * @brief Professional LCD Graphics Demo for JZ2440
 * 
 * Demonstrates LCD initialization, clearing, and primitive drawing (rectangles).
 */

#include "hal/hal_clock.h"
#include "hal/hal_sdram.h"
#include "hal/hal_lcd.h"
#include "hal/hal_gpio.h"

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
    /* 1. Clear screen with a professional background color */
    hal_lcd_clear(COLOR_BLACK);
    
    /* 2. Draw some color blocks */
    hal_lcd_draw_rect(50, 50, 100, 100, COLOR_RED);
    hal_lcd_draw_rect(200, 50, 100, 100, COLOR_GREEN);
    hal_lcd_draw_rect(350, 50, 80, 100, COLOR_YELLOW);
    
    /* 3. Draw a center crosshair */
    /* Horizontal line */
    hal_lcd_draw_rect(0, LCD_HEIGHT/2 - 1, LCD_WIDTH, 2, COLOR_WHITE);
    /* Vertical line */
    hal_lcd_draw_rect(LCD_WIDTH/2 - 1, 0, 2, LCD_HEIGHT, COLOR_WHITE);
}

int main(void) {
    /* 1. Hardware Initialization Sequence */
    hal_clock_init();
    hal_sdram_init(); /* LCD Framebuffer resides in SDRAM */
    hal_lcd_init();

    /* 2. Heartbeat LED setup */
    hal_gpio_init_output(HEARTBEAT_LED);

    /* 3. Graphics Output */
    draw_test_pattern();

    /* 4. Main Loop: Heartbeat */
    while (1) {
        hal_gpio_toggle(HEARTBEAT_LED);
        delay(500000);
    }

    return 0;
}
