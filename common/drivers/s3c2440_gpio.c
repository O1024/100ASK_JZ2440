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
    if (pin < 100) { // GPF 组
        GPFCON &= ~(3 << (pin * 2));
        GPFCON |= (1 << (pin * 2));
    } else { // GPG 组
        int real_pin = pin - 100;
        GPGCON &= ~(3 << (real_pin * 2));
        GPGCON |= (1 << (real_pin * 2));
    }
}

void hal_gpio_init_input(hal_gpio_pin_t pin) {
    if (pin < 100) { // GPF 组
        GPFCON &= ~(3 << (pin * 2)); // 设置为 00 (Input)
        GPFUP  &= ~(1 << pin);       // 开启上拉 (0 表示开启)
    } else { // GPG 组
        int real_pin = pin - 100;
        GPGCON &= ~(3 << (real_pin * 2)); // 设置为 00 (Input)
        GPGUP  &= ~(1 << real_pin);       // 开启上拉
    }
}

hal_gpio_state_t hal_gpio_get(hal_gpio_pin_t pin) {
    if (pin < 100) { // GPF 组
        return (GPFDAT & (1 << pin)) ? GPIO_HIGH : GPIO_LOW;
    } else { // GPG 组
        int real_pin = pin - 100;
        return (GPGDAT & (1 << real_pin)) ? GPIO_HIGH : GPIO_LOW;
    }
}

void hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
    if (pin < 100) { // GPF 组
        if (state == GPIO_HIGH) GPFDAT |= (1 << pin);
        else GPFDAT &= ~(1 << pin);
    } else { // GPG 组
        int real_pin = pin - 100;
        if (state == GPIO_HIGH) GPGDAT |= (1 << real_pin);
        else GPGDAT &= ~(1 << real_pin);
    }
}

void hal_gpio_toggle(hal_gpio_pin_t pin) {
    GPFDAT ^= (1 << pin);
}
