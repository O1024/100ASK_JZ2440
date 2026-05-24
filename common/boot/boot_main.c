/**
 * @file boot_main.c
 * @brief Unified Stage 1 Bootloader Logic (SPL)
 */

#include "hal/hal_clock.h"
#include "hal/hal_sdram.h"
#include "hal/hal_uart.h"
#include <stdint.h>

/* App Entry Point in SDRAM */
#define APP_ENTRY 0x30000000
/* App offset in Flash (after 4KB Bootloader) */
#define APP_FLASH_OFFSET 4096
/* Max size to copy */
#define APP_COPY_SIZE (128 * 1024)

/* Extern declaration for the media-specific relocate function */
extern void relocate_app(uint8_t *dest, uint32_t src_offset, uint32_t len);

void boot_main(void) {
    /* 1. Core Hardware Init */
    hal_clock_init();
    hal_sdram_init();
    
    /* 2. UART for status output */
    hal_uart_init(115200);
    
#if defined(USE_NAND)
    hal_uart_puts("\r\n[Boot] NAND Bootloader Initializing...\r\n");
#elif defined(USE_NOR)
    hal_uart_puts("\r\n[Boot] NOR Bootloader Initializing...\r\n");
#endif

    /* 3. SDRAM Self-test */
    hal_uart_puts("[Boot] SDRAM Test: ");
    volatile uint32_t *sdram = (volatile uint32_t *)APP_ENTRY;
    *sdram = 0x55AA55AA;
    if (*sdram == 0x55AA55AA) {
        hal_uart_puts("OK\r\n");
    } else {
        hal_uart_puts("FAIL!\r\n");
        while(1);
    }

    /* 4. Relocate Main Application to SDRAM */
    hal_uart_puts("[Boot] Relocating app to SDRAM... ");
    relocate_app((uint8_t *)APP_ENTRY, APP_FLASH_OFFSET, APP_COPY_SIZE);
    
    /* 5. Jump to Application */
    volatile uint32_t *app_head = (volatile uint32_t *)APP_ENTRY;
    if ((*app_head & 0xFF000000) == 0xEA000000) {
        hal_uart_puts("DONE\r\n[Boot] Jumping to App...\r\n\r\n");
        void (*app_start)(void) = (void (*)(void))APP_ENTRY;
        app_start();
    } else {
        hal_uart_puts("ERROR: Invalid App Header!\r\n");
        while(1);
    }
}
