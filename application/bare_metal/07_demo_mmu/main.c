/**
 * @file main.c
 * @brief MMU (Memory Management Unit) Demo - NOR/ISRAM Mode
 *
 * Runs from NOR/ISRAM with stack in ISRAM, initializes SDRAM,
 * then enables MMU to map SDRAM via virtual addresses.
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_sdram.h"
#include "s3c2440_soc.h"
#include <stdint.h>

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

#define UART_BAUD_RATE 115200

/* Page table placed in SDRAM (16KB, must be 16KB aligned).
 * ISRAM only has 4KB, not enough for 4096 entries.
 */
#define PAGE_TABLE_BASE  0x30004000U
static uint32_t * const page_table = (uint32_t *)PAGE_TABLE_BASE;

/**
 * @brief Board-level initialization (BSP)
 */
static void bsp_init(void) {
    hal_clock_init();
    hal_uart_init(UART_BAUD_RATE);
    hal_sdram_init();
}

/**
 * @brief Print a hexadecimal value to UART
 */
static void print_hex(uint32_t val) {
    const char *hex = "0123456789ABCDEF";
    char buf[11];
    int i = 10;
    buf[i] = '\0';
    if (val == 0) {
        buf[--i] = '0';
    } else {
        while (val > 0 && i > 0) {
            buf[--i] = hex[val & 0xF];
            val >>= 4;
        }
    }
    hal_uart_puts("0x");
    hal_uart_puts(&buf[i]);
}

/**
 * @brief Setup page tables using section (1MB) descriptors.
 *
 * Identity map critical regions:
 *   0x00000000 - 0x003FFFFF  (NOR / ISRAM code)
 *   0x30000000 - 0x33FFFFFF  (SDRAM)
 *   0x40000000 - 0x40000FFF  (ISRAM stack/data)
 *
 * Plus alias:
 *   Virtual 0xA0000000 -> Physical 0x30000000
 */
static void mmu_setup_page_table(void) {
    /* Section descriptor template:
     *   bits[1:0]  = 10 (section)
     *   bits[3:2]  = 00 (C=0, B=0, uncached/unbuffered)
     *   bit[4]     = 0  (SBZ for ARMv4)
     *   bits[8:5]  = 0  (Domain 0)
     *   bits[11:10]= 11 (AP = read/write, all modes)
     *   bits[31:20]= section base address
     */
    const uint32_t section_template = (3U << 10) | (0U << 5) | (0U << 4) | 2U;

    for (int i = 0; i < 4096; i++) {
        page_table[i] = ((uint32_t)i << 20) | section_template;
    }

    /* Alias: Virtual 0xA0000000 -> Physical 0x30000000 */
    page_table[0xA00] = (0x300U << 20) | section_template;
}

/**
 * @brief Enable MMU, caches, and write buffer
 */
static void mmu_enable(void) {
    uint32_t pt_base = (uint32_t)page_table;

    __asm__ volatile (
        /* Domain 0 = manager (no access permission checks) */
        "mov    r0, #0x3\n"
        "mcr    p15, 0, r0, c3, c0, 0\n"

        /* Set TTBR */
        "mcr    p15, 0, %0, c2, c0, 0\n"

        /* Invalidate TLBs */
        "mcr    p15, 0, r0, c8, c7, 0\n"

        /* Invalidate ICache, DCache, and drain write buffer */
        "mcr    p15, 0, r0, c7, c7, 0\n"
        "mcr    p15, 0, r0, c7, c10, 4\n"

        /* Enable MMU (bit 0), D-Cache (bit 2), write buffer (bit 3), I-Cache (bit 12) */
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "orr    r0, r0, #0x1\n"        /* MMU */
        "orr    r0, r0, #0x4\n"        /* D-Cache */
        "orr    r0, r0, #0x8\n"        /* Write buffer */
        "orr    r0, r0, #0x1000\n"     /* I-Cache */
        "mcr    p15, 0, r0, c1, c0, 0\n"

        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        :
        : "r"(pt_base)
        : "r0"
    );
}

/**
 * @brief Disable MMU
 */
static void mmu_disable(void) {
    __asm__ volatile (
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "bic    r0, r0, #0x1\n"
        "bic    r0, r0, #0x4\n"
        "bic    r0, r0, #0x8\n"
        "bic    r0, r0, #0x1000\n"
        "mcr    p15, 0, r0, c1, c0, 0\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        :
        :
        : "r0"
    );
}

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 1. Board-level Initialization */
    bsp_init();

    /* 2. Initialize C Runtime */
    hal_system_init();

    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("    MMU Demo - ARM920T (S3C2440)        \r\n");
    hal_uart_puts("    Running from NOR/ISRAM               \r\n");
    hal_uart_puts("========================================\r\n");

    /* 3. Write a test pattern to physical SDRAM */
    volatile uint32_t *p_phys = (volatile uint32_t *)0x30000000;
    *p_phys = 0x12345678;

    hal_uart_puts("Test pattern written to phys 0x30000000: ");
    print_hex(*p_phys);
    hal_uart_puts("\r\n");

    /* 4. Setup and enable MMU */
    mmu_setup_page_table();
    hal_uart_puts("Enabling MMU...\r\n");
    mmu_enable();

    /* 5. Read back through virtual alias */
    volatile uint32_t *p_virt = (volatile uint32_t *)0xA0000000;
    uint32_t val = *p_virt;

    hal_uart_puts("Read via virtual addr 0xA0000000: ");
    print_hex(val);
    hal_uart_puts("\r\n");

    if (val == 0x12345678) {
        hal_uart_puts("MMU mapping test: PASSED\r\n");
    } else {
        hal_uart_puts("MMU mapping test: FAILED\r\n");
    }

    /* 6. Disable MMU before halting */
    mmu_disable();
    hal_uart_puts("MMU disabled.\r\n");
    hal_uart_puts("========================================\r\n");

    while (1);
    return 0;
}
