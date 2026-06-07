/**
 * @file hal_uart.c
 * @brief UART HAL Implementation
 *
 * Generic logic layer. Calls LLD primitives.
 */

#include "hal/hal_uart.h"
#include "ll_timer.h"
#include "ll_uart.h"

void hal_uart_init(uint32_t baud_rate) {
    ll_uart_init(baud_rate);
}

void hal_uart_putc(char c) {
    ll_uart_putc(c);
}

char hal_uart_getc(void) {
    return ll_uart_getc();
}

void hal_uart_puts(const char *str) {
    if (!str)
        return;
    while (*str) {
        if (*str == '\n') {
            ll_uart_putc('\r');
        }
        ll_uart_putc(*str++);
    }
}

int hal_uart_tstc(void) {
    return ll_uart_tstc();
}

int hal_uart_getc_timeout(uint32_t timeout_ms, char *c) {
    uint32_t target_ticks = timeout_ms * 31;
    uint16_t start = ll_timer4_get_ticks();
    uint32_t elapsed = 0;

    while (ll_uart_tstc() == 0) {
        uint16_t current = ll_timer4_get_ticks();
        elapsed += (uint16_t)(start - current);
        start = current;
        if (elapsed >= target_ticks) {
            return -1;
        }
    }

    *c = ll_uart_getc();
    return 0;
}

void hal_uart_flush(void) {
    ll_uart_flush();
}

void hal_uart_wait_tx_done(void) {
    ll_uart_wait_tx_done();
}
