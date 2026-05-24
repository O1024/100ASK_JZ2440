/**
 * @file main.c
 * @brief Professional NAND Flash Diagnostic Tool
 * 
 * This example demonstrates:
 * 1. Proper initialization sequence for SDRAM target compatibility.
 * 2. Hardware verification via NAND ID reading.
 * 3. Safe flash manipulation involving bad block checking.
 * 4. A comprehensive Erase-Write-Read-Verify testing cycle.
 */

#include "hal/hal_nand.h"
#include "hal/hal_uart.h"
#include "hal/hal_clock.h"
#include "hal/hal_sdram.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

#define UART_BAUD_RATE 115200

/* Define a test block well away from the bootloader (e.g., Block 10) */
#define TEST_BLOCK      10
#define TEST_PAGE       0
#define TEST_ADDR       (TEST_BLOCK * NAND_BLOCK_SIZE + TEST_PAGE * NAND_PAGE_SIZE)

/* Global buffer placed in RAM (Only one to save space in 4KB ISRAM) */
static uint8_t data_buf[NAND_PAGE_SIZE];

/**
 * @brief Initialize critical hardware for the diagnostic tool
 */
static void hw_init(void) {
    /* 1. Ensure Clock is boosted */
    hal_clock_init();
    
    /* 2. Initialize UART for telemetry */
    hal_uart_init(UART_BAUD_RATE);
    
    /* 3. Initialize NAND Controller */
    hal_nand_init();
}

/**
 * @brief Utility to print an 8-bit hex value
 */
static void print_hex8(uint8_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    hal_uart_putc(hex_chars[(val >> 4) & 0xF]);
    hal_uart_putc(hex_chars[val & 0xF]);
}

/**
 * @brief Utility to print a 32-bit hex value
 */
static void print_hex32(uint32_t val) {
    hal_uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        print_hex8((val >> i) & 0xFF);
    }
}

/**
 * @brief Execute the NAND diagnostic sequence
 */
static void nand_diagnostics(void) {
    uint8_t id_buf[5] = {0};
    int status;
    const char *test_msg = "JZ2440 Professional NAND Diagnostic Pattern: 0x55AA55AA";
    
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("    Professional NAND Diagnostic Tool   \r\n");
    hal_uart_puts("========================================\r\n");

    /* Phase 1: ID Check */
    hal_nand_read_id(id_buf);
    hal_uart_puts("[Phase 1] Chip Identification\r\n");
    hal_uart_puts("  Maker ID  : 0x"); print_hex8(id_buf[0]); hal_uart_puts("\r\n");
    hal_uart_puts("  Device ID : 0x"); print_hex8(id_buf[1]); hal_uart_puts("\r\n");
    hal_uart_puts("  Ext ID    : 0x"); print_hex8(id_buf[2]); print_hex8(id_buf[3]); print_hex8(id_buf[4]); hal_uart_puts("\r\n");

    /* Phase 2: Geometry Report */
    hal_uart_puts("\r\n[Phase 2] Geometry Parameters\r\n");
    hal_uart_puts("  Page Size : 2048 Bytes\r\n");
    hal_uart_puts("  Block Size: 128 KB\r\n");
    hal_uart_puts("  Total Size: 256 MB\r\n");

    /* Phase 3: Bad Block Check */
    hal_uart_puts("\r\n[Phase 3] Bad Block Scan (Target Block ");
    print_hex32(TEST_BLOCK);
    hal_uart_puts(")\r\n");
    
    if (hal_nand_check_bad_block(TEST_BLOCK)) {
        hal_uart_puts("  WARNING: Block is marked as BAD. Aborting test.\r\n");
        return;
    }
    hal_uart_puts("  Status    : Block is GOOD.\r\n");

    /* Phase 4: Integrity Test */
    hal_uart_puts("\r\n[Phase 4] Integrity Test (Erase -> Write -> Read -> Verify)\r\n");
    
    hal_uart_puts("  1. Erasing Block... ");
    status = hal_nand_erase_block(TEST_BLOCK);
    if (status != 0) { hal_uart_puts("FAILED\r\n"); return; }
    hal_uart_puts("OK\r\n");

    /* Prepare test pattern */
    memset(data_buf, 0xFF, NAND_PAGE_SIZE); 
    memcpy(data_buf, test_msg, strlen(test_msg));

    hal_uart_puts("  2. Writing Page...  ");
    status = hal_nand_write_page(TEST_BLOCK, TEST_PAGE, data_buf);
    if (status != 0) { hal_uart_puts("FAILED\r\n"); return; }
    hal_uart_puts("OK\r\n");

    hal_uart_puts("  3. Reading Page...  ");
    memset(data_buf, 0x00, NAND_PAGE_SIZE); /* Clear buffer to ensure we read fresh data */
    status = hal_nand_read_page(TEST_BLOCK, TEST_PAGE, data_buf);
    if (status != 0) { hal_uart_puts("FAILED\r\n"); return; }
    hal_uart_puts("OK\r\n");

    hal_uart_puts("  4. Verifying...     ");
    /* Verify against original message string */
    if (memcmp(data_buf, test_msg, strlen(test_msg)) == 0) {
        hal_uart_puts("PASS\r\n");
        hal_uart_puts("\r\nRESULT: NAND Diagnostic PASSED. Hardware is healthy.\r\n");
    } else {
        hal_uart_puts("FAIL (Data mismatch)\r\n");
        hal_uart_puts("\r\nRESULT: NAND Diagnostic FAILED. Check timing or hardware.\r\n");
    }
    hal_uart_puts("========================================\r\n");
}

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 1. Critical Boot Setup for SDRAM targets */
#ifdef TARGET_SDRAM
    hal_clock_init();
    hal_sdram_init();
#endif

    /* 2. Initialize C Runtime (Data relocation, BSS zeroing) */
    hal_system_init();

    /* 3. Hardware Initialization */
    hw_init();

    /* 4. Run Diagnostics */
    nand_diagnostics();

    /* Halt */
    while (1);
    return 0;
}
