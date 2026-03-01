/**
 * @file hal_gpio.h
 * @brief GPIO HAL Interface for JZ2440 Unified SDK
 * 
 * Copyright (c) 2026 JZ2440 Unified SDK Contributors
 * Distributed under the MIT License.
 */

#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#include <stdint.h>

/**
 * @brief GPIO 引脚定义 (示例)
 */
typedef enum {
    GPF4 = 4,
    GPF5 = 5,
    GPF6 = 6,
    // 其他引脚待后续扩展
} hal_gpio_pin_t;

/**
 * @brief GPIO 电平定义
 */
typedef enum {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1
} hal_gpio_state_t;

/**
 * @brief 初始化 GPIO 引脚为输出模式
 */
void hal_gpio_init_output(hal_gpio_pin_t pin);

/**
 * @brief 设置 GPIO 引脚电平
 */
void hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state);

/**
 * @brief 翻转 GPIO 引脚电平
 */
void hal_gpio_toggle(hal_gpio_pin_t pin);

#endif // __HAL_GPIO_H__
