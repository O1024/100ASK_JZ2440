/**
 * @file s3c2440_timer.c
 * @brief Standard Timer Driver with Overflow Counting
 */

#include "ll_timer.h"
#include "ll_irq.h"
#include "s3c2440_soc.h"
#include <stddef.h>
#include <stdint.h>

/* --- Overflow counting for long-duration measurements --- */
static volatile uint32_t g_timer4_overflows = 0;
static void (*ll_timer4_user_handler)(void) = NULL;

/**
 * @brief Internal ISR: counts overflows and chains to user handler.
 */
static void ll_timer4_internal_isr(void) {
    g_timer4_overflows++;
    if (ll_timer4_user_handler) {
        ll_timer4_user_handler();
    }
}

void ll_timer4_init(uint32_t ms) {
    /* 1. Frequency setup: PCLK (50MHz) / 100 / 16 = 31250 Hz */
    TIMER->TCFG0 &= ~(0xFF << 8);
    TIMER->TCFG0 |= (99 << 8);
    TIMER->TCFG1 &= ~(0xF << 16);
    TIMER->TCFG1 |= (0x3 << 16);

    /* 2. Calculate count for requested ms */
    TIMER->TCNTB4 = (31250 * ms) / 1000;

    /* 3. Manual Update sequence */
    TIMER->TCON |= (1 << 21);
    TIMER->TCON &= ~(1 << 21);

    /* 4. Auto Reload setup */
    TIMER->TCON |= (1 << 22);

    /* 5. Register internal ISR (handles overflow counting + user callback) */
    ll_irq_register(IRQ_TIMER4, ll_timer4_internal_isr);
}

void ll_timer4_start(void) {
    TIMER->TCON |= (1 << 20); /* Kick off */
    ll_irq_enable(IRQ_TIMER4);
}

void ll_timer4_stop(void) {
    TIMER->TCON &= ~(1 << 20);
    ll_irq_disable(IRQ_TIMER4);
}

void ll_timer4_set_handler(void (*handler)(void)) {
    /* Save user callback; internal ISR will call it after counting overflow */
    ll_timer4_user_handler = handler;
}

void ll_timer4_init_freerun(void) {
    /* 1. Frequency setup: PCLK (50MHz) / 100 / 16 = 31250 Hz */
    TIMER->TCFG0 &= ~(0xFF << 8);
    TIMER->TCFG0 |= (99 << 8);
    TIMER->TCFG1 &= ~(0xF << 16);
    TIMER->TCFG1 |= (0x3 << 16);

    /* 2. Set count to maximum (16-bit) */
    TIMER->TCNTB4 = 0xFFFF;

    /* 3. Manual Update sequence */
    TIMER->TCON |= (1 << 21);
    TIMER->TCON &= ~(1 << 21);

    /* 4. Auto Reload setup */
    TIMER->TCON |= (1 << 22);

    /* 5. Register internal ISR for overflow counting */
    ll_irq_register(IRQ_TIMER4, ll_timer4_internal_isr);
}

uint16_t ll_timer4_get_ticks(void) {
    return (uint16_t)TIMER->TCNTO4;
}

void ll_timer4_reset_overflows(void) {
    g_timer4_overflows = 0;
}

uint32_t ll_timer4_get_overflows(void) {
    return g_timer4_overflows;
}
