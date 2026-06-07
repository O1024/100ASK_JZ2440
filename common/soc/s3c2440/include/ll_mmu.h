/**
 * @file ll_mmu.h
 * @brief Low-Level MMU driver for ARM920T (S3C2440)
 */

#ifndef __LL_MMU_H__
#define __LL_MMU_H__

#include <stdint.h>

void ll_mmu_enable(uint32_t page_table_base);
void ll_mmu_disable(void);
int  ll_mmu_is_enabled(void);
void ll_mmu_invalidate_tlb(void);

#endif /* __LL_MMU_H__ */
