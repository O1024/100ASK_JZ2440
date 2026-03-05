/**
 * @file hal_clock.h
 * @brief Clock System HAL Interface for S3C2440
 * 
 * Copyright (c) 2026 JZ2440 Unified SDK Contributors
 * Distributed under the MIT License.
 */

#ifndef __HAL_CLOCK_H__
#define __HAL_CLOCK_H__

/**
 * @brief 初始化系统时钟
 * 
 * 将 FCLK 提升至 400MHz, HCLK 设为 100MHz, PCLK 设为 50MHz
 */
void hal_clock_init(void);

/**
 * @brief 重置系统时钟
 * 
 * 将时钟恢复到默认的 12MHz 旁路模式
 */
void hal_clock_reset(void);

#endif // __HAL_CLOCK_H__
