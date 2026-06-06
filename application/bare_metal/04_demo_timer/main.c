/**
 * @file main.c
 * @brief Timer Interrupt Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_gpio.h"
#include "hal/hal_uart.h"
#include "hal/hal_timer.h"
#include "hal/hal_irq.h"
#include <stdint.h>

extern void hal_system_init(void);

#define HEARTBEAT_LED       BSP_LED1
#define TIMER_INTERVAL_MS   1000

static volatile uint32_t g_timer_ticks = 0;

static void timer4_isr(void) {
    g_timer_ticks++;
    hal_gpio_toggle(HEARTBEAT_LED);
}

static void timer_irq_init(void) {
    hal_irq_init();
    hal_timer4_set_handler(timer4_isr);
    hal_timer4_init(TIMER_INTERVAL_MS);
    hal_timer4_start();
    hal_irq_global_enable();
}

static void print_ticks(uint32_t ticks) {
    hal_uart_puts("[Tick: ");
    hal_uart_putc('0' + (ticks / 100) % 10);
    hal_uart_putc('0' + (ticks / 10) % 10);
    hal_uart_putc('0' + ticks % 10);
    hal_uart_puts("] LED Toggled!\r\n");
}

int main(void) {
    /* 1. C Runtime */
    hal_system_init();
    g_timer_ticks = 0;

    /* 2. Board-level init */
    bsp_clock_init();
    bsp_uart_init();
    bsp_gpio_init();

    BSP_PRINT_BANNER("04 Timer Interrupt Demo");
    hal_uart_puts("Config: Timer4 periodic interrupt every 1000 ms.\r\n");

    timer_irq_init();

    uint32_t last_ticks = 0;
    while (1) {
        uint32_t current_ticks = g_timer_ticks;
        if (current_ticks != last_ticks) {
            last_ticks = current_ticks;
            print_ticks(last_ticks);
        }
    }

    return 0;
}
