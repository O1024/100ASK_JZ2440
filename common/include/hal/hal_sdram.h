/**
 * @file hal_sdram.h
 * @brief Professional SDRAM HAL Interface for JZ2440
 */

#ifndef __HAL_SDRAM_H__
#define __HAL_SDRAM_H__

#include <stdint.h>

/* --- SDRAM Memory Map --- */
#define SDRAM_BASE          0x30000000
#define SDRAM_SIZE          (64 * 1024 * 1024) /* 64MB */
#define SDRAM_END           (SDRAM_BASE + SDRAM_SIZE)

/**
 * @brief Initialize SDRAM Controller (Bank 6)
 * 
 * Configures the memory controller to support 64MB SDRAM on JZ2440.
 * Mandatory before using SDRAM or jumping to code in SDRAM.
 */
void hal_sdram_init(void);

#endif /* __HAL_SDRAM_H__ */
