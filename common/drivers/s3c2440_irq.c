/**
 * @file s3c2440_irq.c
 * @brief Standard Interrupt Controller Driver (Matches Reference Code)
 */

#include "hal/hal_irq.h"
#include <stdint.h>
#include <stddef.h>

#define SRCPND      (*(volatile uint32_t *)0x4A000000)
#define INTMSK      (*(volatile uint32_t *)0x4A000008)
#define INTPND      (*(volatile uint32_t *)0x4A000010)
#define INTOFFSET   (*(volatile uint32_t *)0x4A000014)

static void (*irq_handlers[32])(void);

void hal_irq_init(void) {
    INTMSK = 0xFFFFFFFF;
    SRCPND = 0xFFFFFFFF;
    INTPND = 0xFFFFFFFF;
    for (int i = 0; i < 32; i++) irq_handlers[i] = NULL;
}

void hal_irq_register(int irq_num, void (*handler)(void)) {
    if (irq_num >= 0 && irq_num < 32) irq_handlers[irq_num] = handler;
}

void hal_irq_enable(int irq_num) {
    if (irq_num >= 0 && irq_num < 32) INTMSK &= ~(1 << irq_num);
}

void hal_irq_disable(int irq_num) {
    if (irq_num >= 0 && irq_num < 32) INTMSK |= (1 << irq_num);
}

void hal_irq_global_enable(void) {
    __asm__ volatile ("mrs r0, cpsr\nbic r0, r0, #0x80\nmsr cpsr, r0\n" : : : "r0");
}

void hal_irq_global_disable(void) {
    __asm__ volatile ("mrs r0, cpsr\norr r0, r0, #0x80\nmsr cpsr, r0\n" : : : "r0");
}

void handle_irq(void) {
    uint32_t offset = INTOFFSET;
    if (offset >= 32) return;

    uint32_t bit = (1 << offset);

    /* 1. Call registered handler first */
    if (irq_handlers[offset]) {
        irq_handlers[offset]();
    }

    /* 2. Clear Timer-side status in TINT_CSTAT (bits 5-9) if applicable */
    if (offset >= 10 && offset <= 14) {
        volatile uint32_t *tint_cstat = (volatile uint32_t *)0x51000044;
        uint32_t val = *tint_cstat;
        /* Preserve enable bits (0-4), set the status bit (5-9) to clear it */
        *tint_cstat = (val & 0x1F) | (1 << (offset - 10 + 5));
    }

    /* 3. Clear INTC flags (SRCPND then INTPND) */
    SRCPND = bit;
    INTPND = bit;
}
