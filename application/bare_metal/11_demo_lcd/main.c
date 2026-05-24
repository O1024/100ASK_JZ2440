/**
 * @file main.c
 * @brief Professional LCD Graphics Application (SDRAM Boot Compatible)
 * 
 * This application is designed to be loaded from NAND Flash into SDRAM
 * by a Stage 1/2 bootloader. It demonstrates:
 * 1. High-resolution LCD initialization and color clearing.
 * 2. Basic geometric primitive drawing (rectangles).
 * 3. Execution from external SDRAM (0x30000000).
 * 4. Conditional hardware initialization to support both standalone and
 *    bootloader-launched scenarios.
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_sdram.h"
#include "hal/hal_lcd.h"
#include "hal/hal_gpio.h"
#include "hal/hal_delay.h"
#include <stdint.h>

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

#define HEARTBEAT_LED   GPF4
#define UART_BAUD_RATE  115200

/**
 * @brief Initialize hardware peripherals
 */
static void hw_init(void) {
    /* 
     * Note: If launched from the Professional NOR Bootloader, 
     * Clocks and SDRAM are already configured. We only init
     * what's specific to this app.
     */
    hal_uart_init(UART_BAUD_RATE);
    hal_lcd_init();
    
    hal_gpio_init_output(HEARTBEAT_LED);
    hal_gpio_set(HEARTBEAT_LED, GPIO_HIGH);
}

/**
 * @brief Draw the demo UI on the LCD
 */
static void draw_demo_ui(void) {
    hal_uart_puts("[LCD] Drawing UI elements...\r\n");
    
    /* Clear screen to a professional dark background */
    hal_lcd_clear(COLOR_BLACK);
    
    /* Draw some colorful blocks to verify bit depth and color mapping */
    hal_lcd_draw_rect(50, 50, 100, 100, COLOR_RED);
    hal_lcd_draw_rect(200, 50, 100, 100, COLOR_GREEN);
    hal_lcd_draw_rect(350, 50, 80, 100, COLOR_BLUE);
    
    hal_uart_puts("[LCD] UI Drawing complete.\r\n");
}

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 
     * 1. High-level environment setup.
     * When running from SDRAM, hal_system_init will correctly move 
     * data from Flash(LMA) to SDRAM(VMA) if not already there, 
     * and clear BSS in SDRAM.
     */
    hal_system_init();

    /* 2. Hardware Initialization */
    hw_init();

    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("      JZ2440 Professional LCD Demo      \r\n");
    hal_uart_puts("========================================\r\n");
    hal_uart_puts("Execution: SDRAM (0x30000000)\r\n");

    /* 3. Execute Visual Logic */
    draw_demo_ui();

    /* 4. Main Event Loop */
    hal_uart_puts("[System] Application is running.\r\n");
    while (1) {
        hal_gpio_toggle(HEARTBEAT_LED);
        hal_delay(500000);
    }
    
    return 0;
}
