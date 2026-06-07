/**
 * @file hal_cache.h
 * @brief Hardware Abstraction Layer for ARM920T Cache Control
 */

#ifndef __HAL_CACHE_H__
#define __HAL_CACHE_H__

#include <stdint.h>

/**
 * @brief Enable Instruction Cache (I-Cache)
 */
void hal_cache_enable_icache(void);

/**
 * @brief Disable Instruction Cache (I-Cache)
 */
void hal_cache_disable_icache(void);

/**
 * @brief Check if Instruction Cache is enabled
 * @return 1 if enabled, 0 if disabled
 */
int hal_cache_is_icache_enabled(void);

/**
 * @brief Enable Data Cache (D-Cache)
 */
void hal_cache_enable_dcache(void);

/**
 * @brief Disable Data Cache (D-Cache)
 */
void hal_cache_disable_dcache(void);

/**
 * @brief Check if Data Cache is enabled
 * @return 1 if enabled, 0 if disabled
 */
int hal_cache_is_dcache_enabled(void);

/**
 * @brief Clean D-Cache range (Write back dirty lines to RAM)
 * @param start Start virtual address
 * @param size Size in bytes
 */
void hal_cache_clean_dcache_range(uint32_t start, uint32_t size);

/**
 * @brief Invalidate D-Cache range (Force CPU to re-read from RAM)
 * @param start Start virtual address
 * @param size Size in bytes
 */
void hal_cache_invalidate_dcache_range(uint32_t start, uint32_t size);

#endif /* __HAL_CACHE_H__ */
