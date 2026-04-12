/**
 * @file main.c
 * @brief Professional SDRAM Diagnostic Tool for JZ2440
 * 
 * Verifies external 64MB SDRAM integrity through pattern testing.
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_sdram.h"
#include "hal/hal_gpio.h"

/* --- Configuration --- */
#define TEST_SIZE   (1024 * 1024) /* Test first 1MB for speed, increase if needed */
#define HEARTBEAT   GPF4

/**
 * @brief Simple software delay
 */
static void delay(volatile int count) {
    while (count--) {
        __asm__("nop");
    }
}

/**
 * @brief Comprehensive Memory Test
 * @return 0 on success, -1 on failure
 */
static int sdram_integrity_check(void) {
    volatile uint32_t *p = (volatile uint32_t *)SDRAM_BASE;
    uint32_t count = TEST_SIZE / sizeof(uint32_t);
    
    hal_uart_puts("1. Writing pattern... ");
    for (uint32_t i = 0; i < count; i++) {
        p[i] = i ^ 0xAAAAAAAA;
    }
    hal_uart_puts("DONE\r\n");

    hal_uart_puts("2. Verifying pattern... ");
    for (uint32_t i = 0; i < count; i++) {
        if (p[i] != (i ^ 0xAAAAAAAA)) {
            hal_uart_puts("FAILED at offset ");
            /* Just print 'X' for simple failure indication if hex printing is too much */
            hal_uart_putc('X'); 
            return -1;
        }
    }
    hal_uart_puts("PASSED\r\n");
    
    return 0;
}

int main(void) {
    /* 1. Hardware Initialization */
    hal_clock_init();
    hal_uart_init(115200);
    hal_gpio_init_output(HEARTBEAT);
    
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("     JZ2440 SDRAM DIAGNOSTIC TOOL       \r\n");
    hal_uart_puts("========================================\r\n");

    hal_uart_puts("Initializing SDRAM Controller... ");
    hal_sdram_init();
    hal_uart_puts("OK\r\n");

    hal_uart_puts("Starting Memory Test (1MB scope)...\r\n");
    if (sdram_integrity_check() == 0) {
        hal_uart_puts("\r\n>>> RESULT: SDRAM TEST PASSED <<<\r\n");
    } else {
        hal_uart_puts("\r\n>>> RESULT: SDRAM TEST FAILED <<<\r\n");
    }

    hal_uart_puts("\r\nDiagnostics finished. System idling...\r\n");

    /* 2. Main Loop: Heartbeat */
    while (1) {
        hal_gpio_toggle(HEARTBEAT);
        delay(500000);
    }

    return 0;
}
