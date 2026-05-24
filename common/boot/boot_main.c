/**
 * @file boot_main.c
 * @brief Professional Stage 2 Bootloader (NOR Boot) with YModem Update
 */

#include "hal/hal_clock.h"
#include "hal/hal_sdram.h"
#include "hal/hal_uart.h"
#include "hal/hal_nand.h"
#include "hal/hal_gpio.h"
#include "hal/hal_timer.h"
#include "hal/hal_delay.h"
#include "lib/ymodem.h"
#include <stdint.h>
#include <string.h>

/* App Entry Point in SDRAM */
#define APP_SDRAM_ENTRY 0x30000000
/* App offset in NAND Flash (256KB) */
#define APP_NAND_OFFSET 0x40000
/* Max size to copy from NAND to SDRAM (512KB) */
#define APP_COPY_SIZE   (512 * 1024)

#define BOOT_LED        GPF4
#define UART_BAUD_RATE  115200

/* Global buffer placed in RAM. 
 * Using a single buffer to save space in 4KB ISRAM.
 */
uint8_t boot_data_buf[NAND_PAGE_SIZE];

/**
 * @brief Initialize all required hardware peripherals
 */
static void bsp_init(void) {
    hal_clock_init();
    hal_sdram_init();
    hal_uart_init(UART_BAUD_RATE);
    hal_nand_init();
    hal_gpio_init_output(BOOT_LED);
    hal_gpio_set(BOOT_LED, GPIO_LOW); 
    hal_timer4_init(500);
}

/**
 * @brief YModem callback to write received data directly to NAND Flash
 */
static int nand_write_cb(uint32_t offset, const uint8_t *data, uint32_t length) {
    uint32_t abs_offset = APP_NAND_OFFSET + offset;
    
    /* Erase block if we hit a block boundary */
    if ((abs_offset % NAND_BLOCK_SIZE) == 0) {
        uint32_t block = abs_offset / NAND_BLOCK_SIZE;
        if (hal_nand_check_bad_block(block)) return -1;
        hal_nand_erase_block(block);
    }
    
    /* Write pages. Since YModem chunks might not be page-aligned,
     * we buffer them. However, standard YModem chunks (128/1024) 
     * and NAND pages (2048) are usually compatible.
     */
    uint32_t pages = length / NAND_PAGE_SIZE;
    if (pages == 0 && length > 0) pages = 1; /* Handle smaller chunks */
    
    for (uint32_t i = 0; i < pages; i++) {
        uint32_t page_offset = abs_offset + (i * NAND_PAGE_SIZE);
        uint32_t block = page_offset / NAND_BLOCK_SIZE;
        uint32_t page = (page_offset % NAND_BLOCK_SIZE) / NAND_PAGE_SIZE;
        
        /* Copy to aligned global buffer then write */
        memset(boot_data_buf, 0xFF, NAND_PAGE_SIZE);
        uint32_t chunk_len = (length < NAND_PAGE_SIZE) ? length : NAND_PAGE_SIZE;
        memcpy(boot_data_buf, data + (i * NAND_PAGE_SIZE), chunk_len);

        if (hal_nand_write_page(block, page, boot_data_buf) != 0) {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Enter YModem update mode
 */
static void do_update(void) {
    hal_uart_puts("\r\n[Boot] Enter YModem Update Mode. Please send file...\r\n");
    if (ymodem_receive(nand_write_cb) == YMODEM_OK) {
        hal_uart_puts("\r\n[Boot] Update successful!\r\n");
    } else {
        hal_uart_puts("\r\n[Boot] Update failed or aborted.\r\n");
    }
    hal_uart_puts("[Boot] System halted. Please RESET to boot new firmware.\r\n");
    while(1);
}

/**
 * @brief Load application from NAND to SDRAM
 */
static int load_app_from_nand(void) {
    hal_uart_puts("[Boot] Reading firmware from NAND (Offset: 0x40000) to SDRAM...\r\n");
    int status = hal_nand_read((uint8_t *)APP_SDRAM_ENTRY, APP_NAND_OFFSET, APP_COPY_SIZE);
    if (status == 0) {
        hal_uart_puts("[Boot] Firmware loaded successfully.\r\n");
    } else {
        hal_uart_puts("[Boot] ERROR: Failed to read from NAND Flash!\r\n");
    }
    return status;
}

void boot_main(void) {
    bsp_init();
    
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("   JZ2440 Professional NOR Bootloader   \r\n");
    hal_uart_puts("========================================\r\n");

    /* Boot Menu: Countdown to update mode */
    hal_uart_puts("\r\nHit 'u' to enter update mode within 3 seconds...\r\n");
    int update = 0;
    for (int i = 3; i > 0; i--) {
        hal_uart_putc(i + '0');
        hal_uart_puts(" ");
        /* Poll UART for 1 second */
        for (int j = 0; j < 100; j++) {
            if (hal_uart_tstc()) {
                char c;
                hal_uart_getc_timeout(10, &c);
                if (c == 'u') {
                    update = 1;
                    break;
                }
            }
            hal_delay(10000); /* ~10ms wait */
        }
        if (update) break;
    }
    hal_uart_puts("\r\n");

    if (update) {
        do_update();
    }

    /* Standard Boot Flow */
    if (load_app_from_nand() != 0) {
        hal_uart_puts("[Boot] System Halted.\r\n");
        while(1);
    }

    /* Jump to Application */
    hal_uart_puts("[Boot] Jumping to Application at 0x30000000...\r\n");
    hal_uart_puts("========================================\r\n\r\n");
    
    hal_gpio_set(BOOT_LED, GPIO_HIGH);
    
    void (*app_start)(void) = (void (*)(void))APP_SDRAM_ENTRY;
    app_start();
    
    while(1);
}
