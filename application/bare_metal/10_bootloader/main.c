/**
 * @file boot_main.c
 * @brief Stage 2 Bootloader (NOR Boot) with YModem Update
 */

#include "bsp_init.h"
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

#define BOOT_LED        BSP_LED1

extern void hal_system_init(void);

static uint8_t nand_page_buf[NAND_PAGE_SIZE];
static uint32_t nand_buf_len = 0;
static uint32_t nand_block = 0;
static uint32_t nand_page = 0;

static void nand_write_init(void) {
    nand_buf_len = 0;
    nand_block = APP_NAND_OFFSET / NAND_BLOCK_SIZE;
    nand_page  = (APP_NAND_OFFSET % NAND_BLOCK_SIZE) / NAND_PAGE_SIZE;
}

static int nand_flush_page(void) {
    if (nand_buf_len == 0)
        return 0;

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

static void do_update(void) {
    int result;

    hal_uart_puts("\r\n[Boot] Entering YModem Update Mode...\r\n");

    nand_write_init();

    hal_uart_wait_tx_done();
    hal_uart_flush();
    hal_uart_puts("[Boot] Prepare minicom YModem sender now (Ctrl+A, S, select YModem, pick file).\r\n");
    hal_uart_puts("[Boot] Starting YModem receiver...\r\n");
    hal_uart_wait_tx_done();

    int last_error = 0;
    uint8_t last_seq = 0;
    uint32_t file_size = 0;
    result = ymodem_receive_ex(ymodem_nand_write_cb, ymodem_nand_start_cb, &last_error, &last_seq, &file_size);

    if (result == YMODEM_OK) {
        if (nand_flush_page() != 0) {
            result = YMODEM_ERROR;
        }
    }

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
            case YMODEM_ERR_BAD_HEADER: hal_uart_puts("BAD_HEADER\r\n"); break;
            case YMODEM_ERR_TIMEOUT:    hal_uart_puts("TIMEOUT\r\n"); break;
            case YMODEM_ERR_SEQ:        hal_uart_puts("SEQ mismatch\r\n"); break;
            case YMODEM_ERR_CRC:        hal_uart_puts("CRC mismatch\r\n"); break;
            default:                    hal_uart_puts("Unknown\r\n"); break;
        }
        hal_uart_puts("[Boot] Last seq: ");
        hal_uart_putc("0123456789ABCDEF"[(last_seq >> 4) & 0xF]);
        hal_uart_putc("0123456789ABCDEF"[last_seq & 0xF]);
        hal_uart_puts("\r\n");
        hal_uart_puts("[Boot] Last SOH frame dump:\r\n");
        ymodem_print_last_soh();
    }
    hal_uart_puts("========================================\r\n");

    hal_uart_wait_tx_done();

    if (result == YMODEM_OK) {
        hal_uart_puts("[Boot] Rebooting to bootloader...\r\n");
        hal_uart_wait_tx_done();

        void (*reset)(void) = (void (*)(void))0x00000000;
        reset();
    } else {
        hal_uart_puts("[Boot] Please reset the board.\r\n");
        while (1);
    }
}

static void __attribute__((naked)) switch_to_sdram_stack(void) {
    __asm__ volatile (
        "ldr sp, =0x34000000\n"
        "bx lr\n"
    );
}

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
    /* Bootloader 需要显式初始化所有板载外设 */
    bsp_clock_init();
    bsp_sdram_init();
    bsp_uart_init();
    bsp_nand_init();
    bsp_gpio_init();
    hal_gpio_set(BOOT_LED, GPIO_LOW);

    /* Timer4 作为 UART 超时基准 */
    hal_timer4_init_freerun();
    hal_timer4_start();

    hal_system_init();

    switch_to_sdram_stack();

    BSP_PRINT_BANNER("10 Professional NOR Bootloader");
    hal_uart_puts("Mode   : Stage 2 Bootloader with YModem update\r\n");
    hal_uart_puts("Target : Load app from NAND offset 0x40000 to SDRAM 0x30000000\r\n");

    hal_uart_puts("\r\nPress any key within 3 seconds to enter update mode...\r\n");
    int update = 0;
    for (int i = 3; i > 0; i--) {
        hal_uart_putc(i + '0');
        hal_uart_puts(" ");
        for (int j = 0; j < 100; j++) {
            if (hal_uart_tstc()) {
                char c;
                hal_uart_getc_timeout(10, &c);
                (void)c;
                update = 1;
                break;
            }
            hal_delay(10000);
        }
        if (update) break;
    }
    hal_uart_puts("\r\n");

    if (update) {
        do_update();
    }

    if (load_app_from_nand() != 0) {
        hal_uart_puts("[Boot] System Halted.\r\n");
        while(1);
    }

    hal_uart_puts("[Boot] Jumping to Application at 0x30000000...\r\n");
    hal_uart_puts("========================================\r\n\r\n");

    hal_uart_wait_tx_done();
    hal_gpio_set(BOOT_LED, GPIO_HIGH);

    void (*app_start)(void) = (void (*)(void))APP_SDRAM_ENTRY;
    app_start();

    while(1);
}
