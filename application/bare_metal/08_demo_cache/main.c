/**
 * @file main.c
 * @brief I-Cache Performance Benchmark Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_cache.h"
#include "hal/hal_timer.h"
#include "hal/hal_uart.h"
#include <stdint.h>

extern void hal_system_init(void);

#define ITERATIONS 100000

static void benchmark_test(void) {
    volatile uint32_t a = 0;
    for (uint32_t i = 0; i < ITERATIONS; i++) {
        a += i;
        a ^= 0x55555555;
        a = (a << 1) | (a >> 31);
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

int main(void) {
    bsp_init();
    hal_system_init();

    BSP_PRINT_BANNER("08 I-Cache Benchmark Demo");
    hal_uart_puts("Iterations: ");
    print_dec(ITERATIONS);
    hal_uart_puts("\r\n");

    hal_timer4_init_freerun();
    hal_timer4_start();

    hal_cache_disable_icache();
    hal_uart_puts("Testing with I-Cache OFF... ");
    hal_timer4_reset_overflows();
    uint16_t start_ticks = hal_timer4_get_ticks();
    benchmark_test();
    uint16_t end_ticks = hal_timer4_get_ticks();
    uint32_t off_ticks = hal_timer4_get_elapsed_ticks(start_ticks, end_ticks);
    hal_uart_puts("Done.\r\n");
    hal_uart_puts("Ticks: ");
    print_dec(off_ticks);
    hal_uart_puts("\r\n");

    hal_uart_puts("----------------------------------------\r\n");

    hal_cache_enable_icache();
    hal_uart_puts("Testing with I-Cache ON...  ");
    hal_timer4_reset_overflows();
    start_ticks = hal_timer4_get_ticks();
    benchmark_test();
    end_ticks = hal_timer4_get_ticks();
    uint32_t on_ticks = hal_timer4_get_elapsed_ticks(start_ticks, end_ticks);
    hal_uart_puts("Done.\r\n");
    hal_uart_puts("Ticks: ");
    print_dec(on_ticks);
    hal_uart_puts("\r\n");

    hal_uart_puts("----------------------------------------\r\n");
    if (on_ticks > 0) {
        hal_uart_puts("Performance Gain: ");
        print_dec(off_ticks / on_ticks);
        hal_uart_puts(".");
        print_dec(((off_ticks * 10) / on_ticks) % 10);
        hal_uart_puts("x faster\r\n");
    }
    hal_uart_puts("========================================\r\n");

    while (1)
        ;
    return 0;
}
