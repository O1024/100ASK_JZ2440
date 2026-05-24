/**
 * @file main.c
 * @brief NAND Flash Diagnostic Tool (Clean Pattern)
 */

#include "hal/hal_nand.h"
#include "hal/hal_uart.h"
#include "hal/hal_clock.h"
#include "hal/hal_sdram.h"
#include <stdint.h>
#include <string.h>

/* Boot helper defined in relocate.c */
void hal_system_init(void);

#define NAND_PAGE_SIZE 2048
#define TEST_ADDR      (1024 * 1024)

/* Global buffers go to RAM (ISRAM or SDRAM depending on RAM_TARGET) */
static uint8_t write_buf[NAND_PAGE_SIZE];
static uint8_t read_buf[NAND_PAGE_SIZE];

int main(void) {
    /* 1. If we are targeting SDRAM, we MUST init hardware first 
     * while still running code from Flash.
     */
#ifdef TARGET_SDRAM
    hal_clock_init();
    hal_sdram_init();
#endif

    /* 2. Now RAM is ready, relocate data segments */
    hal_system_init();

    /* 3. Run application logic */
    /* Ensure clock is boosted if not done above */
#ifndef TARGET_SDRAM
    hal_clock_init();
#endif
    
    hal_uart_init(115200);
    hal_nand_init();

    hal_uart_puts("\r\n--- JZ2440 NAND Test (Simplified Boot) ---\r\n");

    /* ... test logic ... */
    const char *msg = "JZ2440 NAND Test Data";
    memcpy(write_buf, msg, strlen(msg) + 1);

    hal_uart_puts("Erasing... ");
    hal_nand_erase(TEST_ADDR);
    hal_uart_puts("Writing... ");
    hal_nand_write(write_buf, TEST_ADDR, NAND_PAGE_SIZE);
    hal_uart_puts("Reading... ");
    hal_nand_read(read_buf, TEST_ADDR, NAND_PAGE_SIZE);

    if (memcmp(read_buf, write_buf, NAND_PAGE_SIZE) == 0) {
        hal_uart_puts("PASSED!\r\nContent: ");
        hal_uart_puts((char *)read_buf);
        hal_uart_puts("\r\n");
    } else {
        hal_uart_puts("FAILED!\r\n");
    }

    while (1);
    return 0;
}
