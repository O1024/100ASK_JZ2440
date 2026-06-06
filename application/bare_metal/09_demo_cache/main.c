/**
 * @file main.c
 * @brief I-Cache Performance Benchmark Demo
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_timer.h"
#include "hal/hal_cache.h"
#include "s3c2440_soc.h"
#include <stdint.h>

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

#define UART_BAUD_RATE 115200
#define ITERATIONS 100000

/**
 * @brief Initialize critical hardware
 */
static void hw_init(void) {
    WDT_CON = 0;
    hal_clock_init();
    hal_uart_init(UART_BAUD_RATE);
}

/**
 * @brief A computation-heavy function to benchmark performance.
 * Use volatile to prevent the compiler from optimizing the loop away.
 */
static void benchmark_test(void) {
    volatile uint32_t a = 0;
    for (uint32_t i = 0; i < ITERATIONS; i++) {
        a += i;
        a ^= 0x55555555;
        a = (a << 1) | (a >> 31);
    }
}

/**
 * @brief Print a decimal value to UART
 */
static void print_dec(uint32_t val) {
    char buf[12];
    int i = 11;
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

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 1. Hardware Initialization */
    hw_init();

    /* 2. Initialize C Runtime */
    hal_system_init();

    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("    I-Cache Performance Benchmark Demo    \r\n");
    hal_uart_puts("========================================\r\n");
    hal_uart_puts("Iterations: "); print_dec(ITERATIONS); hal_uart_puts("\r\n");
    hal_uart_puts("----------------------------------------\r\n");

    /* Initialize Timer 4 for timing */
    hal_timer4_init_freerun();
    hal_timer4_start();

    /* --- Test 1: I-Cache OFF --- */
    hal_cache_disable_icache();
    hal_uart_puts("Testing with I-Cache OFF... ");
    
    uint16_t start_ticks = hal_timer4_get_ticks();
    benchmark_test();
    uint16_t end_ticks = hal_timer4_get_ticks();
    
    uint32_t off_ticks = (start_ticks >= end_ticks) ? (start_ticks - end_ticks) : (0xFFFF - end_ticks + start_ticks + 1);
    hal_uart_puts("Done.\r\n");
    hal_uart_puts("Ticks: "); print_dec(off_ticks); hal_uart_puts("\r\n");

    hal_uart_puts("----------------------------------------\r\n");

    /* --- Test 2: I-Cache ON --- */
    hal_cache_enable_icache();
    hal_uart_puts("Testing with I-Cache ON...  ");
    
    start_ticks = hal_timer4_get_ticks();
    benchmark_test();
    end_ticks = hal_timer4_get_ticks();
    
    uint32_t on_ticks = (start_ticks >= end_ticks) ? (start_ticks - end_ticks) : (0xFFFF - end_ticks + start_ticks + 1);
    hal_uart_puts("Done.\r\n");
    hal_uart_puts("Ticks: "); print_dec(on_ticks); hal_uart_puts("\r\n");

    hal_uart_puts("----------------------------------------\r\n");
    if (on_ticks > 0) {
        hal_uart_puts("Performance Gain: ");
        print_dec(off_ticks / on_ticks);
        hal_uart_puts(".");
        print_dec(((off_ticks * 10) / on_ticks) % 10);
        hal_uart_puts("x faster\r\n");
    }
    hal_uart_puts("========================================\r\n");

    while (1);
    return 0;
}
