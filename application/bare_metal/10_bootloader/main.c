/**
 * @file boot_main.c
 * @brief Professional Stage 2 Bootloader (NOR Boot) with YModem Update
 * @details Ported from STM32 IAP YModem online upgrade
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

#define APP_SDRAM_ENTRY 0x30000000
#define APP_NAND_OFFSET 0x40000
#define APP_COPY_SIZE   (512 * 1024)

#define BOOT_LED        GPF4
#define UART_BAUD_RATE  115200

/* --- NAND Write Buffering for YModem --- */
static uint8_t nand_page_buf[NAND_PAGE_SIZE];
static uint32_t nand_buf_len = 0;
static uint32_t nand_block = 0;
static uint32_t nand_page = 0;

/**
 * @brief Initialize NAND write state
 */
static void nand_write_init(void) {
    nand_buf_len = 0;
    nand_block = APP_NAND_OFFSET / NAND_BLOCK_SIZE;
    nand_page  = (APP_NAND_OFFSET % NAND_BLOCK_SIZE) / NAND_PAGE_SIZE;
}

/**
 * @brief Flush partial page buffer to NAND with 0xFF padding
 * @return 0 on success, -1 on error
 */
static int nand_flush_page(void) {
    if (nand_buf_len == 0)
        return 0;

    /* Pad remaining bytes with 0xFF (erased NAND state) */
    while (nand_buf_len < NAND_PAGE_SIZE) {
        nand_page_buf[nand_buf_len++] = 0xFF;
    }

    if (hal_nand_write_page(nand_block, nand_page, nand_page_buf) != 0) {
        return -1;
    }

    nand_page++;
    if (nand_page >= NAND_PAGES_PER_BLOCK) {
        nand_page = 0;
        nand_block++;
    }
    nand_buf_len = 0;
    return 0;
}

/**
 * @brief YModem write callback: buffers data and flushes to NAND page by page
 */
static int ymodem_nand_write_cb(uint32_t offset, const uint8_t *data, uint32_t length) {
    (void)offset;

    for (uint32_t i = 0; i < length; i++) {
        nand_page_buf[nand_buf_len++] = data[i];
        if (nand_buf_len >= NAND_PAGE_SIZE) {
            if (nand_flush_page() != 0) {
                return -1;
            }
        }
    }
    return 0;
}

/**
 * @brief Erase NAND blocks based on file size (called by YModem before transfer)
 */
static int ymodem_nand_start_cb(uint32_t file_size) {
    uint32_t start_block = APP_NAND_OFFSET / NAND_BLOCK_SIZE;
    uint32_t num_blocks  = (file_size + NAND_BLOCK_SIZE - 1) / NAND_BLOCK_SIZE;

    hal_uart_puts("[Boot] Erasing ");
    hal_uart_putc('0' + (num_blocks / 100) % 10);
    hal_uart_putc('0' + (num_blocks / 10) % 10);
    hal_uart_putc('0' + num_blocks % 10);
    hal_uart_puts(" NAND blocks for ");
    hal_uart_putc('0' + (file_size / 100000) % 10);
    hal_uart_putc('0' + (file_size / 10000) % 10);
    hal_uart_putc('0' + (file_size / 1000) % 10);
    hal_uart_putc('0' + (file_size / 100) % 10);
    hal_uart_putc('0' + (file_size / 10) % 10);
    hal_uart_putc('0' + file_size % 10);
    hal_uart_puts(" bytes...\r\n");

    for (uint32_t i = 0; i < num_blocks; i++) {
        if (hal_nand_erase_block(start_block + i) != 0) {
            return -1;
        }
    }
    return 0;
}

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
 * @brief Perform YModem firmware update to NAND Flash
 * @details Receives application binary via UART YModem and writes to NAND
 */
static void do_update(void) {
    int result;

    hal_uart_puts("\r\n[Boot] Entering YModem Update Mode...\r\n");

    /* Step 1: Initialize write state */
    nand_write_init();

    /* Step 2: Clear UART FIFO and prompt user */
    hal_uart_wait_tx_done();
    hal_uart_flush();
    hal_uart_puts("[Boot] Prepare minicom YModem sender now (Ctrl+A, S, select YModem, pick file).\r\n");
    hal_uart_puts("[Boot] Starting YModem receiver...\r\n");
    hal_uart_wait_tx_done();

    /* Step 4: Receive file via YModem (NAND erase happens in start_cb) */
    int last_error = 0;
    uint8_t last_seq = 0;
    uint32_t file_size = 0;
    result = ymodem_receive_ex(ymodem_nand_write_cb, ymodem_nand_start_cb, &last_error, &last_seq, &file_size);

    /* Step 5: Flush any remaining partial page */
    if (result == YMODEM_OK) {
        if (nand_flush_page() != 0) {
            result = YMODEM_ERROR;
        }
    }

    /* Step 6: Report result (add blank lines to avoid being scrolled away) */
    hal_uart_puts("\r\n\r\n");
    hal_uart_puts("========================================\r\n");
    if (result == YMODEM_OK) {
        hal_uart_puts("[Boot] Firmware update successful!\r\n");
    } else if (result == YMODEM_ABORT) {
        hal_uart_puts("[Boot] Update aborted by sender.\r\n");
    } else {
        hal_uart_puts("[Boot] Update failed!\r\n");
        hal_uart_puts("[Boot] Error detail: ");
        switch (last_error) {
            case YMODEM_ERR_BAD_HEADER:
                hal_uart_puts("BAD_HEADER (not SOH/STX/EOT/CAN)\r\n");
                break;
            case YMODEM_ERR_TIMEOUT:
                hal_uart_puts("TIMEOUT reading packet body\r\n");
                break;
            case YMODEM_ERR_SEQ:
                hal_uart_puts("SEQ mismatch\r\n");
                break;
            case YMODEM_ERR_CRC:
                hal_uart_puts("CRC/Checksum mismatch\r\n");
                break;
            default:
                hal_uart_puts("Unknown\r\n");
                break;
        }
        hal_uart_puts("[Boot] Last seq: ");
        hal_uart_putc("0123456789ABCDEF"[(last_seq >> 4) & 0xF]);
        hal_uart_putc("0123456789ABCDEF"[last_seq & 0xF]);
        hal_uart_puts("\r\n");
        /* Print SOH frame only on failure for debug */
        hal_uart_puts("[Boot] Last SOH frame dump:\r\n");
        ymodem_print_last_soh();
    }
    hal_uart_puts("========================================\r\n");

    hal_uart_wait_tx_done();

    /* Step 7: Reboot on success, or halt on failure */
    if (result == YMODEM_OK) {
        hal_uart_puts("[Boot] Rebooting to bootloader...\r\n");
        hal_uart_wait_tx_done();

        /* Jump to reset vector to restart bootloader */
        void (*reset)(void) = (void (*)(void))0x00000000;
        reset();
    } else {
        hal_uart_puts("[Boot] Please reset the board.\r\n");
        while (1);
    }
}

/**
 * @brief Switch SVC stack from ISRAM to SDRAM
 * @details Called after SDRAM is initialized. Uses naked attribute to avoid
 *          stack operations in the function itself.
 */
static void __attribute__((naked)) switch_to_sdram_stack(void) {
    __asm__ volatile (
        "ldr sp, =0x34000000\n"  /* SDRAM top */
        "bx lr\n"
    );
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

int main(void) {
    bsp_init();

    /* Switch SVC stack to SDRAM after SDRAM is initialized */
    switch_to_sdram_stack();
    hal_uart_puts("[Boot] Stack switched to SDRAM.\r\n");

    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("   JZ2440 Professional NOR Bootloader   \r\n");
    hal_uart_puts("========================================\r\n");

    /* Boot Menu: Countdown to update mode */
    hal_uart_puts("\r\nPress any key within 3 seconds to enter update mode...\r\n");
    int update = 0;
    for (int i = 3; i > 0; i--) {
        hal_uart_putc(i + '0');
        hal_uart_puts(" ");
        /* Poll UART for 1 second */
        for (int j = 0; j < 100; j++) {
            if (hal_uart_tstc()) {
                char c;
                hal_uart_getc_timeout(10, &c);
                (void)c; /* Any key triggers update mode */
                update = 1;
                break;
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
