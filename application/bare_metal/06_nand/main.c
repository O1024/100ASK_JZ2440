/**
 * @file main.c
 * @brief Professional NAND Flash Diagnostic Tool for JZ2440
 * 
 * Includes enhanced hex dump with ASCII preview and structured test sequence.
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_nand.h"
#include "hal/hal_sdram.h"
#include <string.h>

/* Use fixed SDRAM offsets for buffers to avoid any BSS/Relocation issues */
#define WRITE_BUF_ADDR  0x30100000
#define READ_BUF_ADDR   0x30200000

static void hex_dump(const char *title, const uint8_t *buf, uint32_t len) {
    const char hex[] = "0123456789ABCDEF";
    hal_uart_puts("\r\n["); hal_uart_puts(title); hal_uart_puts("]\r\n");
    for (uint32_t i = 0; i < len; i++) {
        hal_uart_putc(hex[buf[i] >> 4]);
        hal_uart_putc(hex[buf[i] & 0xF]);
        if ((i + 1) % 16 == 0) {
            hal_uart_puts("\r\n");
        } else {
            hal_uart_putc(' ');
        }
    }
    hal_uart_puts("\r\n");
}

static void run_nand_diagnostics(void) {
    uint8_t id[5];
    uint8_t *write_buf = (uint8_t *)WRITE_BUF_ADDR;
    uint8_t *read_buf = (uint8_t *)READ_BUF_ADDR;
    uint32_t test_block = 100;

    hal_uart_puts("\r\n1. ID: ");
    hal_nand_read_id(id);
    const char hex_chars[] = "0123456789ABCDEF";
    for (int i = 0; i < 5; i++) {
        hal_uart_putc(hex_chars[id[i] >> 4]);
        hal_uart_putc(hex_chars[id[i] & 0xF]);
        hal_uart_putc(' ');
    }
    hal_uart_puts("\r\n");
    
    hal_uart_puts("2. Check: ");
    if (hal_nand_check_bad_block(test_block)) {
        hal_uart_puts("ERR: Block 100 BAD.\n");
        return;
    }
    hal_uart_puts("GOOD\n");

    hal_uart_puts("3. Erase: ");
    if (hal_nand_erase_block(test_block) != 0) {
        hal_uart_puts("FAILED\n");
        return;
    }
    hal_uart_puts("OK\n");
    
    hal_uart_puts("4. Write: Pattern... ");
    memset(write_buf, 0xEE, NAND_PAGE_SIZE);
    
    const char msg[] = "JZ2440 NAND Test String";
    memcpy(write_buf, msg, strlen(msg));
    
    hal_uart_puts("Ready. Programming... ");
    if (hal_nand_write_page(test_block, 0, write_buf) != 0) {
        hal_uart_puts("FAILED\n");
        return;
    }
    hal_uart_puts("OK\n");

    hal_uart_puts("5. Verify: ");
    memset(read_buf, 0, NAND_PAGE_SIZE);
    hal_nand_read_page(test_block, 0, read_buf);
    
    hex_dump("Data (64B)", read_buf, 64);

    if (memcmp(write_buf, read_buf, NAND_PAGE_SIZE) == 0) {
        hal_uart_puts("SUCCESS\n");
    } else {
        hal_uart_puts("FAILED\n");
    }
}

int main(void) {
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("     JZ2440 NAND DIAGNOSTIC TOOL       \r\n");
    hal_uart_puts("========================================\r\n");
    
    run_nand_diagnostics();

    hal_uart_puts("\r\nDiagnostics finished. System idling...\r\n");
    while (1);
    
    return 0;
}
