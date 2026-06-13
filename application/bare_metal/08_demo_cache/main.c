/**
 * @file main.c
 * @brief Cache (I-Cache + D-Cache + MMU) Performance Benchmark Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_cache.h"
#include "hal/hal_dma.h"
#include "hal/hal_mmu.h"
#include "hal/hal_timer.h"
#include "hal/hal_uart.h"
#include <stdint.h>
#include <string.h>

extern void hal_system_init(void);

#define ITERATIONS     100000
#define MEM_BUF_SIZE   (8 * 1024)
#define MEM_BUF_ADDR   (0x30000000 + 0x10000)

#define PAGE_TABLE_BASE 0x30004000U

static volatile uint8_t *mem_buf = (volatile uint8_t *)MEM_BUF_ADDR;

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

static void print_hex32(uint32_t val) {
    hal_uart_puts("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (val >> (i * 4)) & 0xF;
        hal_uart_putc("0123456789ABCDEF"[nibble]);
    }
}

static void print_hex8(uint8_t val) {
    const char *hex = "0123456789ABCDEF";
    hal_uart_putc(hex[(val >> 4) & 0xF]);
    hal_uart_putc(hex[val & 0xF]);
}

static void dma_coherency_test(int fix_it) {
    /* 
     * src_cache: cached address (0x30400000)
     * src_ram:   same physical address, but non-cached (0x38400000)
     */
    uint8_t *src_cache = (uint8_t *)(0x30400000);
    uint8_t *src_ram   = (uint8_t *)(0x38400000);
    uint8_t *dst_cache = (uint8_t *)(0x30500000);
    uint8_t *dst_ram   = (uint8_t *)(0x38500000);
    uint32_t len = 32;

    hal_uart_puts(fix_it ? "\r\n[Scenario: WITH Cache Maintenance]\r\n"
                         : "\r\n[Scenario: WITHOUT Cache Maintenance]\r\n");

    /* 1. Init RAM with 0xAA using the Non-cached address */
    for (uint32_t i = 0; i < len; i++) {
        src_ram[i] = 0xAA;
        dst_ram[i] = 0x00;
    }
    /* Ensure the Cached view is also clean and matches RAM */
    hal_cache_invalidate_dcache_range((uint32_t)src_cache, len);
    hal_cache_invalidate_dcache_range((uint32_t)dst_cache, len);

    /* 
     * IMPORTANT: ARM920T is Read-Allocate ONLY. 
     * We must READ first to bring the line into Cache. 
     * If we don't, the following write will bypass Cache and go to RAM!
     */
    volatile uint8_t dummy;
    for (uint32_t i = 0; i < len; i++) {
        dummy = src_cache[i];
    }
    (void)dummy;

    /* 2. CPU writes 0x55 to Cached address */
    hal_uart_puts("Step 1: CPU writes 0x55 to src (Cached).\r\n");
    for (uint32_t i = 0; i < len; i++) {
        src_cache[i] = 0x55;
    }

    /* 3. Deep Verification: What's actually in RAM vs Cache? */
    hal_uart_puts("   Check: CPU reads src (Cached)     = 0x");
    print_hex8(src_cache[0]);
    hal_uart_puts("\r\n   Check: Direct read RAM (NC alias) = 0x");
    print_hex8(src_ram[0]);
    
    if (src_ram[0] == 0x55) {
        hal_uart_puts(" (WARN: Cache is acting as Write-Through!)\r\n");
    } else {
        hal_uart_puts(" (GOOD: Cache is holding 0x55, RAM still has 0xAA!)\r\n");
    }

    /* 4. Fix or Skip */
    if (fix_it) {
        hal_uart_puts("Step 2: Cleaning Cache...\r\n");
        hal_cache_clean_dcache_range((uint32_t)src_cache, len);
        hal_uart_puts("   Check: After Clean, RAM (NC) = 0x");
        print_hex8(src_ram[0]);
        hal_uart_puts("\r\n");
    }

    /* 5. DMA Transfer (Always uses Physical RAM) */
    hal_uart_puts("Step 3: DMA transferring...\r\n");
    hal_dma_config_t dma_cfg = {
        .src_addr = 0x30400000, /* Physical address of src */
        .src_bus = DMA_BUS_AHB,
        .src_addr_mode = DMA_ADDR_INC,
        .dst_addr = 0x30500000, /* Physical address of dst */
        .dst_bus = DMA_BUS_AHB,
        .dst_addr_mode = DMA_ADDR_INC,
        .data_size = DMA_DATA_BYTE,
        .trans_type = DMA_TRANS_UNIT,
        .transfer_count = len
    };
    hal_dma_config_software(DMA_CH0, &dma_cfg);
    hal_dma_start(DMA_CH0);
    while (hal_dma_is_busy(DMA_CH0));

    if (fix_it) {
        hal_cache_invalidate_dcache_range((uint32_t)dst_cache, len);
    }

    /* 6. Verify result */
    hal_uart_puts("Step 4: Result in dst[0] (Cached view): 0x");
    print_hex8(dst_cache[0]);
    if (dst_cache[0] == 0x55) {
        hal_uart_puts(" (SUCCESS)\r\n");
    } else {
        hal_uart_puts(" (FAILED: Inconsistency detected!)\r\n");
    }
}

static void mmu_setup_page_table(void) {
    uint32_t *page_table = (uint32_t *)PAGE_TABLE_BASE;

    const uint32_t cacheable_section = (3U << 10) | (1U << 4) | (1U << 3) | (1U << 2) | 2U;
    const uint32_t non_cacheable_section = (3U << 10) | (1U << 4) | 2U;

    /* Identity map 0x00000000 - 0x47FFFFFF (Includes Code, SDRAM, and Internal SRAM) */
    for (int i = 0; i < 1152; i++) {
        page_table[i] = ((uint32_t)i << 20) | cacheable_section;
    }

    /* 
     * Special Mapping for Coherency Test:
     * Virtual 0x38000000 -> Physical 0x30000000 (Non-cacheable)
     */
    for (int i = 0; i < 128; i++) {
        page_table[0x380 + i] = ((uint32_t)(0x300 + i) << 20) | non_cacheable_section;
    }

    /* Identity map peripherals 0x48000000+ (Non-cacheable) */
    for (int i = 1152; i < 4096; i++) {
        page_table[i] = ((uint32_t)i << 20) | non_cacheable_section;
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

int main(void) {
    hal_system_init();
    bsp_init();

    BSP_PRINT_BANNER("08 Cache (I-Cache + D-Cache) Benchmark Demo");
    
    /* Debug: Check CP15 Control Register 1 */
    uint32_t cp15_c1;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(cp15_c1));
    hal_uart_puts("CP15 Control Reg 1: ");
    print_hex32(cp15_c1);
    hal_uart_puts("\r\n");

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

    /* --------------------------------------------------
     * DMA Coherency Demo
     * -------------------------------------------------- */
    print_section_header("DMA Coherency Demo");
    hal_uart_puts("MMU + D-Cache are ENABLED.\r\n");

    /* Test 1: Fail scenario */
    dma_coherency_test(0);

    /* Test 2: Success scenario */
    dma_coherency_test(1);

    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("Benchmark Complete.\r\n");
    hal_uart_puts("========================================\r\n");

    while (1)
        ;
    return 0;
}
