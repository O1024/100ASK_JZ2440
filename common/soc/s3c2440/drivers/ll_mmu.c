/**
 * @file s3c2440_mmu.c
 * @brief MMU driver implementation for ARM920T (S3C2440)
 *
 * Provides low-level MMU control via CP15 co-processor.
 */

#include "ll_mmu.h"
#include <stdint.h>

void ll_mmu_enable(uint32_t page_table_base) {
    uint32_t control;
    uint32_t domain = 0x3;
    uint32_t zero = 0;

    __asm__ volatile(
        /* Set domain access control: Domain 0 = Manager */
        "mcr    p15, 0, %3, c3, c0, 0\n"

        /* Set translation table base */
        "mcr    p15, 0, %1, c2, c0, 0\n"

        /* Invalidate TLB */
        "mcr    p15, 0, %2, c8, c7, 0\n"

        /* Invalidate caches */
        "mcr    p15, 0, %2, c7, c7, 0\n"

        /* Drain write buffer */
        "mcr    p15, 0, %2, c7, c10, 4\n"

        /* Enable MMU + D-Cache + I-Cache */
        /* Alignment Check (A bit) is disabled for better performance with unaligned accesses */
        "mrc    p15, 0, %0, c1, c0, 0\n"
        "orr    %0, %0, #0x1\n"       /* M bit: MMU enable */
        "orr    %0, %0, #0x4\n"       /* D bit: Data cache enable */
        "orr    %0, %0, #0x1000\n"    /* I bit: Instruction cache enable */
        "mcr    p15, 0, %0, c1, c0, 0\n"

        /* CP15 pipeline flush */
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        : "=&r"(control)
        : "r"(page_table_base), "r"(zero), "r"(domain)
        : "memory");
}

void ll_mmu_disable(void) {
    __asm__ volatile(
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "bic    r0, r0, #0x1\n"       /* Clear M bit */
        "bic    r0, r0, #0x4\n"       /* Clear D bit */
        "bic    r0, r0, #0x8\n"       /* Clear A bit */
        "bic    r0, r0, #0x1000\n"    /* Clear I bit */
        "mcr    p15, 0, r0, c1, c0, 0\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        :
        :
        : "r0");
}

int ll_mmu_is_enabled(void) {
    uint32_t reg;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0\n" : "=r"(reg));
    return (reg & 0x1) ? 1 : 0;
}

void ll_mmu_invalidate_tlb(void) {
    uint32_t zero = 0;
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 0\n" : : "r"(zero));
}
