/**
 * @file spl_nor_main.c
 * @brief High-level SPL logic for hardware init and NOR relocation
 */

#include "hal/hal_clock.h"
#include "hal/hal_sdram.h"
#include "hal/hal_uart.h"
#include <stdint.h>

/* Pointer to the linked entry point of Stage 2 (Main App) */
#define STAGE2_ENTRY 0x30000000
/* Assume the main application is at 4KB offset in NOR (after SPL) */
#define NOR_APP_OFFSET 4096
/* Read up to 128KB into SDRAM */
#define APP_LOAD_SIZE   (128 * 1024)

void spl_nor_main(void) {
    /* 1. Initialize Core Hardware */
    hal_clock_init();
    hal_sdram_init();
    
    /* 2. Initialize UART */
    hal_uart_init(115200);
    hal_uart_puts("\r\n[SPL-NOR] JZ2440 Stage 1 Bootloader\r\n");
    
    /* Early SDRAM Check */
    hal_uart_puts("[SPL-NOR] SDRAM Self-test: ");
    volatile uint32_t *sdram = (volatile uint32_t *)STAGE2_ENTRY;
    *sdram = 0x87654321;
    if (*sdram == 0x87654321) {
        hal_uart_puts("PASSED\r\n");
    } else {
        hal_uart_puts("FAILED!\r\n");
        while(1);
    }

    /* 3. Load Main Application from NOR to SDRAM */
    hal_uart_puts("[SPL-NOR] Relocating App from NOR (4K) to SDRAM (0x30000000)... ");
    
    uint32_t *src = (uint32_t *)NOR_APP_OFFSET;
    uint32_t *dest = (uint32_t *)STAGE2_ENTRY;
    uint32_t words = APP_LOAD_SIZE / 4;
    
    for (uint32_t i = 0; i < words; i++) {
        dest[i] = src[i];
    }
    
    /* Quick Verification */
    if ((*dest & 0xFF000000) != 0xEA000000) {
        hal_uart_puts("FAILED!\r\n");
        hal_uart_puts("[SPL-NOR] Error: Valid Stage 2 header not found.\r\n");
        while(1);
    }
    hal_uart_puts("DONE\r\n");

    /* 4. Jump to Stage 2 Entry Point */
    hal_uart_puts("[SPL-NOR] Jumping to Stage 2...\r\n\r\n");
    
    void (*app_start)(void) = (void (*)(void))STAGE2_ENTRY;
    app_start();
}
