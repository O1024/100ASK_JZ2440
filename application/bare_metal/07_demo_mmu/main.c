/**
 * @file main.c
 * @brief MMU Demo using BSP + HAL
 *
 * Demonstrates virtual-to-physical address mapping using the HAL MMU API.
 * Virtual 0xA0000000 is mapped to Physical 0x30000000 (SDRAM base).
 */

#include "bsp_init.h"
#include "hal/hal_mmu.h"
#include "hal/hal_uart.h"
#include <stdint.h>

extern void hal_system_init(void);

#define PAGE_TABLE_BASE 0x30004000U
static uint32_t *const page_table = (uint32_t *)PAGE_TABLE_BASE;

static void print_hex(uint32_t val) {
    const char *hex = "0123456789ABCDEF";
    char        buf[11];
    int         i = 10;
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
    /*
     * Section descriptor template (1MB sections):
     *   Bits[1:0]  = 10  (Section)
     *   Bit[4]     = 1   (Always 1)
     *   Bits[11:10]= 11  (AP=3, Full access)
     */
    const uint32_t section_template = (3U << 10) | (0U << 5) | (0U << 4) | 2U;

    /* Identity map entire 4GB address space */
    for (int i = 0; i < 4096; i++) {
        page_table[i] = ((uint32_t)i << 20) | section_template;
    }

    /* Remap: Virtual 0xA0000000 -> Physical 0x30000000 */
    page_table[0xA00] = (0x300U << 20) | section_template;
}

int main(void) {
    bsp_init();
    hal_system_init();

    BSP_PRINT_BANNER("07 MMU Demo (using HAL API)");
    hal_uart_puts("Mode   : Running from NOR/ISRAM\r\n");
    hal_uart_puts("Target : Virtual 0xA0000000 -> Physical 0x30000000\r\n");

    volatile uint32_t *p_phys = (volatile uint32_t *)BSP_SDRAM_BASE;
    *p_phys = 0x12345678;

    hal_uart_puts("Test pattern written to phys 0x30000000: ");
    print_hex(*p_phys);
    hal_uart_puts("\r\n");

    mmu_setup_page_table();
    hal_uart_puts("Enabling MMU via HAL...\r\n");
    hal_mmu_enable(PAGE_TABLE_BASE);

    volatile uint32_t *p_virt = (volatile uint32_t *)0xA0000000;
    uint32_t           val = *p_virt;

    hal_uart_puts("Read via virtual addr 0xA0000000: ");
    print_hex(val);
    hal_uart_puts("\r\n");

    if (val == 0x12345678) {
        hal_uart_puts("MMU mapping test: PASSED\r\n");
    } else {
        hal_uart_puts("MMU mapping test: FAILED\r\n");
    }

    hal_uart_puts("MMU status: ");
    hal_uart_puts(hal_mmu_is_enabled() ? "Enabled\r\n" : "Disabled\r\n");

    hal_mmu_disable();
    hal_uart_puts("MMU disabled via HAL.\r\n");
    hal_uart_puts("========================================\r\n");

    while (1)
        ;
    return 0;
}
