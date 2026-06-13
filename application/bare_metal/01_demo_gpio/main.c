/**
 * @file main.c
 * @brief GPIO Demo - LED & Key Input
 */

#include "bsp_init.h"
#include "hal/hal_delay.h"
#include "hal/hal_gpio.h"
#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

extern void hal_system_init(void);

typedef struct {
    hal_gpio_pin_t key_pin;
    hal_gpio_pin_t led_pin;
} key_led_map_t;

static const key_led_map_t mappings[] = {
    {BSP_KEY3, BSP_LED1},
    {BSP_KEY2, BSP_LED2},
    {BSP_KEY1, BSP_LED3},
};

static void key_scan_loop(void) {
    while (1) {
        for (size_t i = 0; i < ARRAY_SIZE(mappings); i++) {
            hal_gpio_state_t state1 = hal_gpio_get(mappings[i].key_pin);

            if (state1 == GPIO_LOW) {
                hal_delay(1000);
                hal_gpio_state_t state2 = hal_gpio_get(mappings[i].key_pin);

                if (state2 == GPIO_LOW) {
                    hal_gpio_set(mappings[i].led_pin, GPIO_LOW);
                }
            } else {
                hal_gpio_set(mappings[i].led_pin, GPIO_HIGH);
            }
        }
    }
}

int main(void) {
    /* 1. C Runtime */
    hal_system_init();

    /* 2. Board-level init */
    bsp_init();

    /* 3. Key scan loop */
    key_scan_loop();

    return 0;
}
