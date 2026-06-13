/**
 * @file main.c
 * @brief SDRAM Diagnostic Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_uart.h"
#include <stddef.h>
#include <stdint.h>

extern void hal_system_init(void);

static void print_hex32(uint32_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    hal_uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        hal_uart_putc(hex_chars[(val >> i) & 0xF]);
    }
}

static int sdram_test_chunk(uint32_t start_addr, uint32_t size_bytes, uint32_t pattern) {
    volatile uint32_t *p = (volatile uint32_t *)start_addr;
    uint32_t           count = size_bytes / 4;

    hal_uart_puts("  Writing pattern ");
    print_hex32(pattern);
    hal_uart_puts("... ");

    for (uint32_t i = 0; i < count; i++) {
        p[i] = pattern;
    }

    hal_uart_puts("Verifying... ");
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

int main(void) {
    /* 1. C Runtime */
    hal_system_init();

    /* 2. Board-level init */
    bsp_init();

    BSP_PRINT_BANNER("05 SDRAM Diagnostic Demo");
    hal_uart_puts("Base Address : ");
    print_hex32(BSP_SDRAM_BASE);
    hal_uart_puts("\r\n");
    hal_uart_puts("Total Size   : 64 MB\r\n");
    hal_uart_puts("Test Scope   : First 1 MB with 4 patterns\r\n");

    uint32_t test_size = 1024 * 1024;
    int      errors = 0;

    errors += sdram_test_chunk(BSP_SDRAM_BASE, test_size, 0x55555555);
    errors += sdram_test_chunk(BSP_SDRAM_BASE, test_size, 0xAAAAAAAA);
    errors += sdram_test_chunk(BSP_SDRAM_BASE, test_size, 0x00000000);
    errors += sdram_test_chunk(BSP_SDRAM_BASE, test_size, 0xFFFFFFFF);

    hal_uart_puts("----------------------------------------\r\n");
    if (errors == 0) {
        hal_uart_puts("RESULT: SDRAM Diagnostic PASSED.\r\n");
    } else {
        hal_uart_puts("RESULT: SDRAM Diagnostic FAILED.\r\n");
    }
    hal_uart_puts("========================================\r\n");

    while (1)
        ;
    return 0;
}
