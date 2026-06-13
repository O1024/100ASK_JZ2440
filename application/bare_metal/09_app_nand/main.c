/**
 * @file main.c
 * @brief NAND Flash Diagnostic Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_nand.h"
#include "hal/hal_uart.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern void hal_system_init(void);

#define TEST_BLOCK 10
#define TEST_PAGE  0
#define TEST_ADDR  (TEST_BLOCK * NAND_BLOCK_SIZE + TEST_PAGE * NAND_PAGE_SIZE)

static uint8_t data_buf[NAND_PAGE_SIZE];

static void print_hex8(uint8_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    hal_uart_putc(hex_chars[(val >> 4) & 0xF]);
    hal_uart_putc(hex_chars[val & 0xF]);
}

static void print_hex32(uint32_t val) {
    hal_uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        print_hex8((val >> i) & 0xFF);
    }
}

static void nand_diagnostics(void) {
    uint8_t     id_buf[5] = {0};
    int         status;
    const char *test_msg = "JZ2440 Professional NAND Diagnostic Pattern: 0x55AA55AA";

    BSP_PRINT_BANNER("09 NAND Flash Diagnostic Demo");
    hal_uart_puts("Test Block: ");
    print_hex32(TEST_BLOCK);
    hal_uart_puts("\r\n");

    hal_nand_read_id(id_buf);
    hal_uart_puts("[Phase 1] Chip Identification\r\n");
    hal_uart_puts("  Maker ID  : 0x");
    print_hex8(id_buf[0]);
    hal_uart_puts("\r\n");
    hal_uart_puts("  Device ID : 0x");
    print_hex8(id_buf[1]);
    hal_uart_puts("\r\n");
    hal_uart_puts("  Ext ID    : 0x");
    print_hex8(id_buf[2]);
    print_hex8(id_buf[3]);
    print_hex8(id_buf[4]);
    hal_uart_puts("\r\n");

    hal_uart_puts("\r\n[Phase 2] Geometry Parameters\r\n");
    hal_uart_puts("  Page Size : 2048 Bytes\r\n");
    hal_uart_puts("  Block Size: 128 KB\r\n");
    hal_uart_puts("  Total Size: 256 MB\r\n");

    hal_uart_puts("\r\n[Phase 3] Bad Block Scan\r\n");

    if (hal_nand_check_bad_block(TEST_BLOCK)) {
        hal_uart_puts("  WARNING: Block is BAD. Aborting test.\r\n");
        return;
    }
    hal_uart_puts("  Status    : Block is GOOD.\r\n");

    hal_uart_puts("\r\n[Phase 4] Integrity Test\r\n");

    hal_uart_puts("  1. Erasing Block... ");
    status = hal_nand_erase_block(TEST_BLOCK);
    if (status != 0) {
        hal_uart_puts("FAILED\r\n");
        return;
    }
    hal_uart_puts("OK\r\n");

    memset(data_buf, 0xFF, NAND_PAGE_SIZE);
    memcpy(data_buf, test_msg, strlen(test_msg));

    hal_uart_puts("  2. Writing Page...  ");
    status = hal_nand_write_page(TEST_BLOCK, TEST_PAGE, data_buf);
    if (status != 0) {
        hal_uart_puts("FAILED\r\n");
        return;
    }
    hal_uart_puts("OK\r\n");

    hal_uart_puts("  3. Reading Page...  ");
    memset(data_buf, 0x00, NAND_PAGE_SIZE);
    status = hal_nand_read_page(TEST_BLOCK, TEST_PAGE, data_buf);
    if (status != 0) {
        hal_uart_puts("FAILED\r\n");
        return;
    }
    hal_uart_puts("OK\r\n");

    hal_uart_puts("  4. Verifying...     ");
    if (memcmp(data_buf, test_msg, strlen(test_msg)) == 0) {
        hal_uart_puts("PASS\r\n");
        hal_uart_puts("\r\nRESULT: NAND Diagnostic PASSED.\r\n");
    } else {
        hal_uart_puts("FAIL (Data mismatch)\r\n");
        hal_uart_puts("\r\nRESULT: NAND Diagnostic FAILED.\r\n");
    }
    hal_uart_puts("========================================\r\n");
}

int main(void) {
#ifdef TARGET_SDRAM
    bsp_clock_init();
    bsp_sdram_init();
#endif

    hal_system_init();

#ifdef TARGET_SDRAM
    bsp_uart_init();
#else
    bsp_init();
#endif
    bsp_nand_init();

    nand_diagnostics();

    while (1)
        ;
    return 0;
}
