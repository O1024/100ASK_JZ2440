/**
 * @file hal_mmu.h
 * @brief Hardware Abstraction Layer for ARM920T MMU Control
 */

#ifndef __HAL_MMU_H__
#define __HAL_MMU_H__

#include <stdint.h>

/**
 * @brief Initialize and enable MMU with 1:1 identity mapping
 * @param page_table_base Physical address of page table (must be 16KB aligned)
 *
 * This sets up section entries (1MB each) for the entire 4GB address space.
 * All regions are mapped as Strongly-Ordered (AP=3, Domain=0).
 * SDRAM region (0x30000000-0x38000000) is additionally marked as Cacheable.
 */
void hal_mmu_enable(uint32_t page_table_base);

/**
 * @brief Disable MMU
 */
void hal_mmu_disable(void);

/**
 * @brief Check if MMU is enabled
 * @return 1 if enabled, 0 if disabled
 */
int hal_mmu_is_enabled(void);

/**
 * @brief Invalidate TLB
 */
void hal_mmu_invalidate_tlb(void);

#endif /* __HAL_MMU_H__ */
