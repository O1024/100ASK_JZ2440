/**
 * @file main.c
 * @brief Cache (I-Cache + D-Cache + MMU) Performance Benchmark Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_cache.h"
#include "hal/hal_mmu.h"
#include "hal/hal_timer.h"
#include "hal/hal_uart.h"
#include <stdint.h>

extern void hal_system_init(void);

#define ITERATIONS     100000
#define MEM_BUF_SIZE   (8 * 1024)
#define MEM_BUF_ADDR   (0x30000000 + 0x10000)

#define PAGE_TABLE_BASE 0x30004000U

static volatile uint8_t *mem_buf = (volatile uint8_t *)MEM_BUF_ADDR;

static void mmu_setup_page_table(void) {
    uint32_t *page_table = (uint32_t *)PAGE_TABLE_BASE;

    /*
     * Section descriptor template:
     *   Bit[1:0]  = 10  (Section, 1MB)
     *   Bit[2]    = 1   (B: Bufferable)
     *   Bit[3]    = 1   (C: Cacheable)
     *   Bit[4]    = 1   (Always 1 for section)
     *   Bit[8:5]  = 0000 (Domain 0)
     *   Bit[11:10]= 11  (AP=3, Full access)
     */
    const uint32_t cacheable_section = (3U << 10) | (1U << 4) | (1U << 3) | (1U << 2) | 2U;
    const uint32_t non_cacheable_section = (3U << 10) | (1U << 4) | 2U;

    /* Identity map entire 4GB address space as cacheable by default */
    /* Note: In a real system, peripheral areas (0x48000000+) should be non-cacheable */
    for (int i = 0; i < 4096; i++) {
        if (i >= 0x480) {
            /* Peripheral space: Memory Controllers, UART, Timer, etc. */
            page_table[i] = ((uint32_t)i << 20) | non_cacheable_section;
        } else {
            /* Code and RAM areas */
            page_table[i] = ((uint32_t)i << 20) | cacheable_section;
        }
    }
}

static void benchmark_icache_test(void) {
    volatile uint32_t a = 0;
    for (uint32_t i = 0; i < ITERATIONS; i++) {
        a += i;
        a ^= 0x55555555;
        a = (a << 1) | (a >> 31);
    }
}

static void benchmark_dcache_test(void) {
    volatile uint8_t *buf = mem_buf;
    uint32_t           sz = MEM_BUF_SIZE;

    for (uint32_t n = 0; n < 100; n++) {
        for (uint32_t i = 0; i < sz; i++) {
            buf[i] = (uint8_t)(i + n);
        }
        uint8_t sum = 0;
        for (uint32_t i = 0; i < sz; i++) {
            sum += buf[i];
        }
        (void)sum;
    }
}

static void print_dec(uint32_t val) {
    char buf[12];
    int  i = 11;
    buf[i] = '\0';
    if (val == 0) {
        buf[--i] = '0';
    } else {
        while (val > 0 && i > 0) {
            buf[--i] = (val % 10) + '0';
            val /= 10;
        }
    }
    hal_uart_puts(&buf[i]);
}

static void print_section_header(const char *title) {
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts(title);
    hal_uart_puts("\r\n========================================\r\n");
}

int main(void) {
    bsp_init();
    hal_system_init();

    BSP_PRINT_BANNER("08 Cache (I-Cache + D-Cache) Benchmark Demo");
    hal_uart_puts("I-Cache Iterations: ");
    print_dec(ITERATIONS);
    hal_uart_puts("\r\nD-Cache Mem Size:   ");
    print_dec(MEM_BUF_SIZE);
    hal_uart_puts(" bytes\r\n");

    hal_timer4_init_freerun();
    hal_timer4_start();

    /* --------------------------------------------------
     * I-Cache Benchmark
     * -------------------------------------------------- */
    print_section_header("I-Cache Benchmark");

    hal_cache_disable_icache();
    hal_uart_puts("I-Cache OFF... ");
    hal_timer4_reset_overflows();
    uint16_t start_ticks = hal_timer4_get_ticks();
    benchmark_icache_test();
    uint16_t end_ticks = hal_timer4_get_ticks();
    uint32_t icache_off_ticks = hal_timer4_get_elapsed_ticks(start_ticks, end_ticks);
    hal_uart_puts("Done. Ticks: ");
    print_dec(icache_off_ticks);
    hal_uart_puts("\r\n");

    hal_cache_enable_icache();
    hal_uart_puts("I-Cache ON...  ");
    hal_timer4_reset_overflows();
    start_ticks = hal_timer4_get_ticks();
    benchmark_icache_test();
    end_ticks = hal_timer4_get_ticks();
    uint32_t icache_on_ticks = hal_timer4_get_elapsed_ticks(start_ticks, end_ticks);
    hal_uart_puts("Done. Ticks: ");
    print_dec(icache_on_ticks);
    hal_uart_puts("\r\n");

    if (icache_on_ticks > 0) {
        hal_uart_puts("I-Cache Gain:   ");
        print_dec(icache_off_ticks / icache_on_ticks);
        hal_uart_puts(".");
        print_dec(((icache_off_ticks * 10) / icache_on_ticks) % 10);
        hal_uart_puts("x faster\r\n");
    }

    /* --------------------------------------------------
     * D-Cache Benchmark (requires MMU)
     * -------------------------------------------------- */
    print_section_header("D-Cache Benchmark (with MMU)");

    /* Setup MMU page table before enabling D-Cache */
    hal_uart_puts("Setting up MMU page table...\r\n");
    mmu_setup_page_table();

    hal_uart_puts("D-Cache OFF (MMU disabled)... ");
    hal_timer4_reset_overflows();
    start_ticks = hal_timer4_get_ticks();
    benchmark_dcache_test();
    end_ticks = hal_timer4_get_ticks();
    uint32_t dcache_off_ticks = hal_timer4_get_elapsed_ticks(start_ticks, end_ticks);
    hal_uart_puts("Done. Ticks: ");
    print_dec(dcache_off_ticks);
    hal_uart_puts("\r\n");

    /* Enable MMU + D-Cache together */
    hal_uart_puts("Enabling MMU + D-Cache...\r\n");
    hal_mmu_enable(PAGE_TABLE_BASE);

    /* Verify MMU and D-Cache are actually enabled */
    hal_uart_puts("MMU Status: ");
    hal_uart_puts(hal_mmu_is_enabled() ? "Enabled\r\n" : "DISABLED!\r\n");
    hal_uart_puts("D-Cache Status: ");
    hal_uart_puts(hal_cache_is_dcache_enabled() ? "Enabled\r\n" : "DISABLED!\r\n");

    hal_uart_puts("D-Cache ON (MMU enabled)...  ");
    hal_timer4_reset_overflows();
    start_ticks = hal_timer4_get_ticks();
    benchmark_dcache_test();
    end_ticks = hal_timer4_get_ticks();
    uint32_t dcache_on_ticks = hal_timer4_get_elapsed_ticks(start_ticks, end_ticks);
    hal_uart_puts("Done. Ticks: ");
    print_dec(dcache_on_ticks);
    hal_uart_puts("\r\n");

    if (dcache_on_ticks > 0) {
        if (dcache_on_ticks < dcache_off_ticks) {
            hal_uart_puts("D-Cache Gain:   ");
            print_dec(dcache_off_ticks / dcache_on_ticks);
            hal_uart_puts(".");
            print_dec(((dcache_off_ticks * 10) / dcache_on_ticks) % 10);
            hal_uart_puts("x faster\r\n");
        } else {
            hal_uart_puts("D-Cache Loss:   ");
            print_dec(dcache_on_ticks / dcache_off_ticks);
            hal_uart_puts(".");
            print_dec(((dcache_on_ticks * 10) / dcache_off_ticks) % 10);
            hal_uart_puts("x slower (check page table)\r\n");
        }
    }

    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("Benchmark Complete.\r\n");
    hal_uart_puts("========================================\r\n");

    while (1)
        ;
    return 0;
}
