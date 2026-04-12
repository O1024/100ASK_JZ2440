/**
 * @file spl_main.c
 * @brief High-level SPL logic for hardware init and NAND relocation
 */

#include "hal/hal_clock.h"
#include "hal/hal_sdram.h"
#include "hal/hal_uart.h"
#include "hal/hal_nand.h"
#include <stdint.h>

/* Pointer to the linked entry point of Stage 2 (Main App) */
#define STAGE2_ENTRY 0x30000000
/* Assume the main application is at 4KB offset in NAND (after SPL) */
#define NAND_APP_OFFSET 4096
/* Read up to 128KB into SDRAM (plenty for our current experiments) */
#define APP_LOAD_SIZE   (128 * 1024)

void spl_main(void) {
    /* 1. Initialize Core Hardware */
    hal_clock_init();
    hal_sdram_init();
    hal_nand_init();
    
    /* 2. Initialize UART for debug output (115200 8N1) */
    hal_uart_init(115200);
    hal_uart_puts("\r\n[SPL] JZ2440 Stage 1 Bootloader\r\n");
    
    /* Early SDRAM Check: Write a magic value and read it back */
    hal_uart_puts("[SPL] SDRAM Self-test: ");
    volatile uint32_t *sdram = (volatile uint32_t *)STAGE2_ENTRY;
    *sdram = 0x12345678;
    if (*sdram == 0x12345678) {
        hal_uart_puts("PASSED\r\n");
    } else {
        hal_uart_puts("FAILED!\r\n");
        hal_uart_puts("[SPL] Error: SDRAM not working correctly.\r\n");
        while(1);
    }

    /* 3. Load Main Application from NAND to SDRAM */
    hal_uart_puts("[SPL] Loading App from NAND (4K) to SDRAM (0x30000000)... ");
    hal_nand_read((uint8_t *)STAGE2_ENTRY, NAND_APP_OFFSET, APP_LOAD_SIZE);
    
    /* Quick Verification: Stage 2 should start with a branch instruction (0xEAxxxxxx) */
    volatile uint32_t *stage2 = (volatile uint32_t *)STAGE2_ENTRY;
    if ((*stage2 & 0xFF000000) != 0xEA000000) {
        hal_uart_puts("FAILED!\r\n");
        hal_uart_puts("[SPL] Data at 0x30000000: ");
        const char hex_chars[] = "0123456789ABCDEF";
        for (int i = 0; i < 4; i++) {
            uint32_t val = stage2[i];
            for (int j = 7; j >= 0; j--) {
                hal_uart_putc(hex_chars[(val >> (j * 4)) & 0xF]);
            }
            hal_uart_putc(' ');
        }
        hal_uart_puts("\r\n[SPL] Error: Valid Stage 2 header not found.\r\n");
        while(1);
    }
    hal_uart_puts("DONE\r\n");

    /* 4. Jump to Stage 2 Entry Point (Absolute Jump) */
    hal_uart_puts("[SPL] Jumping to Stage 2...\r\n\r\n");
    
    void (*app_start)(void) = (void (*)(void))STAGE2_ENTRY;
    app_start();
}
