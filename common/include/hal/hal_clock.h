/**
 * @file hal_clock.h
 * @brief Clock HAL Interface for JZ2440
 */

#ifndef __HAL_CLOCK_H__
#define __HAL_CLOCK_H__

#include <stdint.h>

/**
 * @brief 初始化时钟系统 (默认 FCLK=400MHz, HCLK=100MHz, PCLK=50MHz)
 */
void hal_clock_init(void);

/**
 * @brief 重置时钟为默认频率 (12MHz)
 */
void hal_clock_reset(void);

/**
 * @brief 获取当前 PCLK 频率 (单位: Hz)
 */
uint32_t hal_clock_get_pclk(void);

#endif // __HAL_CLOCK_H__
