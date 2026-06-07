/**
 * @file s3c2440_cache.c
 * @brief Cache driver implementation for ARM920T (S3C2440)
 */

#include "ll_cache.h"
#include <stdint.h>

void ll_cache_enable_icache(void) {
    __asm__ volatile("mrc p15, 0, r0, c1, c0, 0\n" /* Read Control Register 1 */
                     "orr r0, r0, #(1 << 12)\n"    /* Set bit 12 (I-Cache) */
                     "mcr p15, 0, r0, c1, c0, 0\n" /* Write back */
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     :
                     :
                     : "r0");
}

void ll_cache_disable_icache(void) {
    __asm__ volatile("mrc p15, 0, r0, c1, c0, 0\n" /* Read Control Register 1 */
                     "bic r0, r0, #(1 << 12)\n"    /* Clear bit 12 (I-Cache) */
                     "mcr p15, 0, r0, c1, c0, 0\n" /* Write back */
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     :
                     :
                     : "r0");
}

int ll_cache_is_icache_enabled(void) {
    uint32_t reg;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0\n" : "=r"(reg));
    return (reg & (1 << 12)) ? 1 : 0;
}

void ll_cache_enable_dcache(void) {
    __asm__ volatile("mrc p15, 0, r0, c1, c0, 0\n" /* Read Control Register 1 */
                     "orr r0, r0, #(1 << 2)\n"     /* Set bit 2 (D-Cache) */
                     "mcr p15, 0, r0, c1, c0, 0\n" /* Write back */
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     :
                     :
                     : "r0");
}

void ll_cache_disable_dcache(void) {
    __asm__ volatile("mrc p15, 0, r0, c1, c0, 0\n" /* Read Control Register 1 */
                     "bic r0, r0, #(1 << 2)\n"     /* Clear bit 2 (D-Cache) */
                     "mcr p15, 0, r0, c1, c0, 0\n" /* Write back */
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     "nop\n"
                     :
                     :
                     : "r0");
}

int ll_cache_is_dcache_enabled(void) {
    uint32_t reg;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0\n" : "=r"(reg));
    return (reg & (1 << 2)) ? 1 : 0;
}
