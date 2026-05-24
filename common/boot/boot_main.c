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

/* Use external string utilities from common/lib/string.c */
extern void *memset(void *s, int c, uint32_t n);
extern void *memcpy(void *dest, const void *src, uint32_t n);
extern void hal_system_init(void);

#define APP_SDRAM_ENTRY 0x30000000
#define APP_NAND_OFFSET 0x40000
#define APP_COPY_SIZE   (512 * 1024)

#define BOOT_LED        GPF4
#define UART_BAUD_RATE  115200

/* Global buffers placed in RAM. 
 * Accumulates 1024-byte YModem chunks into 2048-byte NAND pages.
 */
static uint8_t nand_page_buf[NAND_PAGE_SIZE];
static uint32_t nand_buf_offset = 0;
static uint32_t total_received = 0;

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
    
    /* Initialize Timer 4 as a hardware timebase for UART timeouts */
    hal_timer4_init_freerun();
    /* Manually start Timer 4 (Bit 20) without triggering hal_irq_enable */
    volatile uint32_t *tcon = (volatile uint32_t *)0x51000008;
    *tcon |= (1 << 20);
}

/**
 * @brief YModem callback: Accumulate chunks and write 2048-byte pages
 */
static int nand_write_cb(uint32_t offset, const uint8_t *data, uint32_t length) {
    /* Initialize buffers on first data packet (offset 0) */
    if (offset == 0) {
        nand_buf_offset = 0;
        total_received = 0;
        memset(nand_page_buf, 0xFF, NAND_PAGE_SIZE);
    }
    
    for (uint32_t i = 0; i < length; i++) {
        nand_page_buf[nand_buf_offset++] = data[i];
        
        /* Once we have a full NAND page, write it */
        if (nand_buf_offset == NAND_PAGE_SIZE) {
            /* Calculate absolute NAND address for this page */
            uint32_t page_abs_offset = APP_NAND_OFFSET + total_received + i + 1 - NAND_PAGE_SIZE;
            uint32_t block = page_abs_offset / NAND_BLOCK_SIZE;
            uint32_t page = (page_abs_offset % NAND_BLOCK_SIZE) / NAND_PAGE_SIZE;
            
            /* Erase block if we are writing the first page of the block */
            if (page == 0) {
                if (hal_nand_check_bad_block(block)) return -1;
                hal_nand_erase_block(block);
            }
            
            if (hal_nand_write_page(block, page, nand_page_buf) != 0) {
                return -1;
            }
            
            /* Reset buffer for next page */
            nand_buf_offset = 0;
            memset(nand_page_buf, 0xFF, NAND_PAGE_SIZE);
        }
    }
    total_received += length;
    return 0;
}

/**
 * @brief Flush any remaining data in the buffer to NAND at end of transfer
 */
static void flush_nand_buf(void) {
    if (nand_buf_offset > 0) {
        uint32_t page_abs_offset = APP_NAND_OFFSET + total_received - nand_buf_offset;
        uint32_t block = page_abs_offset / NAND_BLOCK_SIZE;
        uint32_t page = (page_abs_offset % NAND_BLOCK_SIZE) / NAND_PAGE_SIZE;
        
        /* If this is the start of a block that hasn't been erased yet */
        if (page == 0) {
            if (hal_nand_check_bad_block(block)) return;
            hal_nand_erase_block(block);
        }
        
        hal_nand_write_page(block, page, nand_page_buf);
        nand_buf_offset = 0;
    }
}

extern void ymodem_print_trace(void);

/**
 * @brief Enter YModem update mode
 */
static void do_update(void) {
    hal_uart_puts("\r\n[Boot] Enter YModem Update Mode. Please send file...\r\n");
    
    /* Ensure TX is fully sent before flushing RX and starting protocol */
    hal_uart_wait_tx_done();
    hal_uart_flush();
    
    if (ymodem_receive(nand_write_cb) == YMODEM_OK) {
        flush_nand_buf(); /* Write the final partial page */
        hal_uart_puts("\r\n[Boot] Update successful!\r\n");
    } else {
        /* Wait 1.5 seconds to let Minicom 'sb' process exit back to terminal */
        hal_delay(1500000); 
        hal_uart_puts("\r\n[Boot] Update failed or aborted.\r\n");
        ymodem_print_trace();
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
    
    /* Ensure last bootloader messages are fully sent */
    hal_uart_wait_tx_done();
    hal_gpio_set(BOOT_LED, GPIO_HIGH);
    
    void (*app_start)(void) = (void (*)(void))APP_SDRAM_ENTRY;
    app_start();
    
    while(1);
}
