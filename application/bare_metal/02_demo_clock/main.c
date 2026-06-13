/**
 * @file main.c
 * @brief Clock Configuration Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_delay.h"
#include "hal/hal_gpio.h"
#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

extern void hal_system_init(void);

static const hal_gpio_pin_t led_pins[] = {BSP_LED1, BSP_LED2, BSP_LED3};

static void led_demo(void) {
    while (1) {
        for (size_t i = 0; i < ARRAY_SIZE(led_pins); i++) {
            hal_gpio_set(led_pins[i], GPIO_LOW);
            hal_delay(100000);
            hal_gpio_set(led_pins[i], GPIO_HIGH);
        }
    }
}

int main(void) {
    /* 1. C Runtime */
    hal_system_init();

    /* 2. Board-level init */
    bsp_init();

    /* 3. Demo loop */
    led_demo();

    return 0;
}
