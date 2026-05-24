/**
 * @file s3c2440_timer.c
 * @brief Standard Timer Driver
 */

#include "hal/hal_timer.h"
#include "hal/hal_irq.h"
#include "s3c2440_soc.h"
#include <stdint.h>

void hal_timer4_init(uint32_t ms) {
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

    /* 5. Clear status (Bit 9) and Enable Timer 4 interrupt locally in TINT_CSTAT (Bit 4) */
    TIMER->TINT_CSTAT = (TIMER->TINT_CSTAT & 0x1F) | (1 << 4) | (1 << 9);
}

void hal_timer4_start(void) {
    TIMER->TCON |= (1 << 20);      /* Kick off */
    hal_irq_enable(IRQ_TIMER4);
}

void hal_timer4_stop(void) {
    TIMER->TCON &= ~(1 << 20);
    hal_irq_disable(IRQ_TIMER4);
}

void hal_timer4_set_handler(void (*handler)(void)) {
    hal_irq_register(IRQ_TIMER4, handler);
}

void hal_timer4_init_freerun(void) {
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
    
    /* 4. Auto Reload setup (No Interrupts enabled in TINT_CSTAT) */
    TIMER->TCON |= (1 << 22);
}

uint16_t hal_timer4_get_ticks(void) {
    return (uint16_t)TIMER->TCNTO4;
}
