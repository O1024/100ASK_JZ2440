/**
 * @file main.c
 * @brief GPIO Demo - LED & Key Input
 *
 * Demonstrates the 5-layer architecture:
 *   Application -> BSP -> HAL -> LLD -> SoC
 *
 * Features:
 *   - LED output control
 *   - Key input with software debouncing
 *   - Key-to-LED mapping (按下按键点亮对应LED)
 */

#include "bsp_init.h"
#include "hal/hal_delay.h"
#include "hal/hal_gpio.h"
#include <stdint.h>
#include <stddef.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

extern void hal_system_init(void);

/**
 * @brief Key-to-LED mapping for JZ2440 board
 */
typedef struct {
    hal_gpio_pin_t key_pin;
    hal_gpio_pin_t led_pin;
} key_led_map_t;

static const key_led_map_t mappings[] = {
    {BSP_KEY3, BSP_LED1},   /* S4 -> LED1 */
    {BSP_KEY2, BSP_LED2},   /* S3 -> LED2 */
    {BSP_KEY1, BSP_LED3},   /* S2 -> LED3 */
};

/**
 * @brief Key scan with software debouncing
 */
static void key_scan_loop(void) {
    while (1) {
        for (size_t i = 0; i < ARRAY_SIZE(mappings); i++) {
            hal_gpio_state_t state1 = hal_gpio_get(mappings[i].key_pin);

            if (state1 == GPIO_LOW) {  /* Key pressed (active low) */
                hal_delay(1000);       /* Debounce */
                hal_gpio_state_t state2 = hal_gpio_get(mappings[i].key_pin);

                if (state2 == GPIO_LOW) {
                    hal_gpio_set(mappings[i].led_pin, GPIO_LOW);  /* LED ON */
                }
            } else {
                hal_gpio_set(mappings[i].led_pin, GPIO_HIGH);     /* LED OFF */
            }
        }
    }
}

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 1. C Runtime */
    hal_system_init();

    /* 2. Board-level GPIO initialization (LED + Key) */
    bsp_gpio_init();

    /* 3. Key scan loop */
    key_scan_loop();

    return 0;
}
