/**
 * @file main.c
 * @brief Professional SDRAM Diagnostic Tool
 * 
 * This example demonstrates:
 * 1. Critical SDRAM controller initialization.
 * 2. A robust memory testing algorithm using alternating bit patterns 
 *    (0x55555555 and 0xAAAAAAAA) to detect stuck bits and adjacent line shorts.
 * 3. Clear UART reporting of diagnostic progress and results.
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_sdram.h"
#include <stdint.h>
#include <stddef.h>

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

#define UART_BAUD_RATE 115200

/**
 * @brief Initialize critical hardware for the diagnostic tool
 */
static void hw_init(void) {
    /* Clock MUST be boosted to 400MHz before SDRAM init for correct timing */
    hal_clock_init();
    
    /* Initialize the Memory Controller for Bank 6 (SDRAM) */
    hal_sdram_init();
    
    /* Initialize UART for reporting */
    hal_uart_init(UART_BAUD_RATE);
}

/**
 * @brief Print a 32-bit hex value to UART
 */
static void print_hex32(uint32_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    hal_uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        hal_uart_putc(hex_chars[(val >> i) & 0xF]);
    }
}

/**
 * @brief Test a specific memory chunk with a given pattern
 * 
 * @param start_addr Starting physical address
 * @param size_bytes Size of the chunk to test
 * @param pattern 32-bit pattern to write and verify
 * @return 0 on success, 1 on failure
 */
static int sdram_test_chunk(uint32_t start_addr, uint32_t size_bytes, uint32_t pattern) {
    volatile uint32_t *p = (volatile uint32_t *)start_addr;
    uint32_t count = size_bytes / 4;
    
    hal_uart_puts("  Writing pattern ");
    print_hex32(pattern);
    hal_uart_puts("... ");

    /* Write Phase */
    for (uint32_t i = 0; i < count; i++) {
        p[i] = pattern;
    }

    hal_uart_puts("Verifying... ");

    /* Read/Verify Phase */
    for (uint32_t i = 0; i < count; i++) {
        if (p[i] != pattern) {
            hal_uart_puts("FAIL at address ");
            print_hex32((uint32_t)&p[i]);
            hal_uart_puts("\r\n");
            return 1;
        }
    }
    
    hal_uart_puts("PASS\r\n");
    return 0;
}

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 1. Hardware Initialization (Crucial to do this before relocation) */
    hw_init();

    /* 2. Initialize C Runtime (Data relocation, BSS zeroing) */
    hal_system_init();

    /* --- Full Environment Ready --- */
    
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("    Professional SDRAM Diagnostic Tool    \r\n");
    hal_uart_puts("========================================\r\n");
    hal_uart_puts("Base Address : "); print_hex32(SDRAM_BASE); hal_uart_puts("\r\n");
    hal_uart_puts("Total Size   : 64 MB\r\n");
    hal_uart_puts("----------------------------------------\r\n");
    
    /* 
     * Test a 1MB chunk at the beginning of SDRAM. 
     * Testing the full 64MB via software loop can be time-consuming,
     * so we test a representative block for fast diagnostics.
     */
    uint32_t test_size = 1024 * 1024; /* 1MB */
    
    hal_uart_puts("Starting Fast Diagnostic (First 1MB)...\r\n");
    
    int errors = 0;
    
    /* Pattern 1: Alternating bits (0101...) */
    errors += sdram_test_chunk(SDRAM_BASE, test_size, 0x55555555);
    
    /* Pattern 2: Alternating bits (1010...) */
    errors += sdram_test_chunk(SDRAM_BASE, test_size, 0xAAAAAAAA);
    
    /* Pattern 3: All Zeros */
    errors += sdram_test_chunk(SDRAM_BASE, test_size, 0x00000000);
    
    /* Pattern 4: All Ones */
    errors += sdram_test_chunk(SDRAM_BASE, test_size, 0xFFFFFFFF);

    hal_uart_puts("----------------------------------------\r\n");
    if (errors == 0) {
        hal_uart_puts("RESULT: SDRAM Diagnostic PASSED. Hardware is healthy.\r\n");
    } else {
        hal_uart_puts("RESULT: SDRAM Diagnostic FAILED. Check hardware/timing.\r\n");
    }
    hal_uart_puts("========================================\r\n");

    /* Halt */
    while (1);
    return 0;
}
