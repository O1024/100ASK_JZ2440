/**
 * @file hal_mmu.c
 * @brief HAL wrapper for MMU operations
 */

#include "hal/hal_mmu.h"
#include "ll_mmu.h"

void hal_mmu_enable(uint32_t page_table_base) {
    ll_mmu_enable(page_table_base);
}

void hal_mmu_disable(void) {
    ll_mmu_disable();
}

int hal_mmu_is_enabled(void) {
    return ll_mmu_is_enabled();
}

void hal_mmu_invalidate_tlb(void) {
    ll_mmu_invalidate_tlb();
}
