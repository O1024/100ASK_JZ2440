/**
 * @file s3c2440_timer.c
 * @brief Standard Timer Driver
 */

#include "ll_timer.h"
#include "ll_irq.h"
#include "s3c2440_soc.h"
#include <stdint.h>

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

    /*
     * S3C2440A 没有 TINT_CSTAT 寄存器。
     * Timer4 中断使能通过主 INTC (INTMSK) 控制，
     * 中断状态清除通过 SRCPND/INTPND 完成。
     */
}

void ll_timer4_start(void) {
    TIMER->TCON |= (1 << 20);      /* Kick off */
    ll_irq_enable(IRQ_TIMER4);
}

void ll_timer4_stop(void) {
    TIMER->TCON &= ~(1 << 20);
    ll_irq_disable(IRQ_TIMER4);
}

void ll_timer4_set_handler(void (*handler)(void)) {
    ll_irq_register(IRQ_TIMER4, handler);
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

    /*
     * S3C2440A 没有 TINT_CSTAT 寄存器。
     * Free-run 模式不启用中断，无需额外清除操作。
     */
}

uint16_t ll_timer4_get_ticks(void) {
    return (uint16_t)TIMER->TCNTO4;
}
