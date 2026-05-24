/**
 * @file hal_delay.h
 * @brief Simple software delay loop interface
 */

#ifndef __HAL_DELAY_H__
#define __HAL_DELAY_H__

#include <stdint.h>

/**
 * @brief Busy wait delay loop
 * @param count Number of iterations
 */
void hal_delay(volatile uint32_t count);

#endif /* __HAL_DELAY_H__ */
