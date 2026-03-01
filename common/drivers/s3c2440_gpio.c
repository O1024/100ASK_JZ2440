/**
 * @file s3c2440_gpio.c
 * @brief S3C2440 GPIO Driver Implementation
 * 
 * Copyright (c) 2026 JZ2440 Unified SDK Contributors
 * Distributed under the MIT License.
 */

#include "hal/hal_gpio.h"
#include "s3c2440_soc.h"

void hal_gpio_init_output(hal_gpio_pin_t pin) {
    // [2n+1:2n] 设为 01 即为输出模式
    GPFCON &= ~(3 << (pin * 2));
    GPFCON |= (1 << (pin * 2));
}

void hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
    if (state == GPIO_HIGH) {
        GPFDAT |= (1 << pin);
    } else {
        GPFDAT &= ~(1 << pin);
    }
}

void hal_gpio_toggle(hal_gpio_pin_t pin) {
    GPFDAT ^= (1 << pin);
}
