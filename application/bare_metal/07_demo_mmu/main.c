/**
 * @file main.c
 * @brief MMU Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_uart.h"
#include <stdint.h>

extern void hal_system_init(void);

#define PAGE_TABLE_BASE  0x30004000U
static uint32_t * const page_table = (uint32_t *)PAGE_TABLE_BASE;

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

static void mmu_setup_page_table(void) {
    const uint32_t section_template = (3U << 10) | (0U << 5) | (0U << 4) | 2U;

    for (int i = 0; i < 4096; i++) {
        page_table[i] = ((uint32_t)i << 20) | section_template;
    }

    page_table[0xA00] = (0x300U << 20) | section_template;
}

static void mmu_enable(void) {
    uint32_t pt_base = (uint32_t)page_table;

    __asm__ volatile (
        "mov    r0, #0x3\n"
        "mcr    p15, 0, r0, c3, c0, 0\n"
        "mcr    p15, 0, %0, c2, c0, 0\n"
        "mcr    p15, 0, r0, c8, c7, 0\n"
        "mcr    p15, 0, r0, c7, c7, 0\n"
        "mcr    p15, 0, r0, c7, c10, 4\n"
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "orr    r0, r0, #0x1\n"
        "orr    r0, r0, #0x4\n"
        "orr    r0, r0, #0x8\n"
        "orr    r0, r0, #0x1000\n"
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

int main(void) {
    bsp_init();
    hal_system_init();

    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("    MMU Demo - ARM920T (S3C2440)        \r\n");
    hal_uart_puts("    Running from NOR/ISRAM               \r\n");
    hal_uart_puts("========================================\r\n");

    volatile uint32_t *p_phys = (volatile uint32_t *)BSP_SDRAM_BASE;
    *p_phys = 0x12345678;

    hal_uart_puts("Test pattern written to phys 0x30000000: ");
    print_hex(*p_phys);
    hal_uart_puts("\r\n");

    mmu_setup_page_table();
    hal_uart_puts("Enabling MMU...\r\n");
    mmu_enable();

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

    mmu_disable();
    hal_uart_puts("MMU disabled.\r\n");
    hal_uart_puts("========================================\r\n");

    while (1);
    return 0;
}
