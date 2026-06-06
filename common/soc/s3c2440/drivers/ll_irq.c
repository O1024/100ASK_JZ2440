/**
 * @file s3c2440_irq.c
 * @brief Standard Interrupt Controller Driver (Matches Reference Code)
 */

#include "ll_irq.h"
#include <stdint.h>
#include <stddef.h>

#define SRCPND      (*(volatile uint32_t *)0x4A000000)
#define INTMSK      (*(volatile uint32_t *)0x4A000008)
#define INTPND      (*(volatile uint32_t *)0x4A000010)
#define INTOFFSET   (*(volatile uint32_t *)0x4A000014)

static void (*irq_handlers[32])(void);

void ll_irq_init(void) {
    INTMSK = 0xFFFFFFFF;
    SRCPND = 0xFFFFFFFF;
    INTPND = 0xFFFFFFFF;
    for (int i = 0; i < 32; i++) irq_handlers[i] = NULL;
}

void ll_irq_register(int irq_num, void (*handler)(void)) {
    if (irq_num >= 0 && irq_num < 32) irq_handlers[irq_num] = handler;
}

void ll_irq_enable(int irq_num) {
    if (irq_num >= 0 && irq_num < 32) INTMSK &= ~(1 << irq_num);
}

void ll_irq_disable(int irq_num) {
    if (irq_num >= 0 && irq_num < 32) INTMSK |= (1 << irq_num);
}

void ll_irq_global_enable(void) {
    __asm__ volatile ("mrs r0, cpsr\nbic r0, r0, #0x80\nmsr cpsr, r0\n" : : : "r0");
}

void ll_irq_global_disable(void) {
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

    /*
     * S3C2440A 没有 TINT_CSTAT 寄存器。
     * Timer 中断清除仅通过主 INTC 的 SRCPND/INTPND 完成。
     */

    /* 2. Clear INTC flags (SRCPND then INTPND) */
    SRCPND = bit;
    INTPND = bit;
}
