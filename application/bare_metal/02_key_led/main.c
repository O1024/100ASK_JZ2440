/**
 * @file main.c
 * @brief Key Control LED Application for JZ2440
 * 
 * Professional version: Mapping keys to LEDs using structured configuration.
 */

#include "hal/hal_gpio.h"

/* --- Hardware Mapping --- */
typedef struct {
    hal_gpio_pin_t key_pin;
    hal_gpio_pin_t led_pin;
} key_led_map_t;

static const key_led_map_t mappings[] = {
    {GPF0, GPF4}, /* S2 -> LED4 */
    {GPF2, GPF5}, /* S3 -> LED5 */
    {GPG3, GPF6}, /* S4 -> LED6 (If available on GPG3) */
};

#define MAP_COUNT (sizeof(mappings) / sizeof(mappings[0]))

int main(void) {
    /* 1. Hardware Initialization */
    for (int i = 0; i < MAP_COUNT; i++) {
        /* Configure LEDs: Output, default HIGH (OFF) */
        hal_gpio_init_output(mappings[i].led_pin);
        hal_gpio_set(mappings[i].led_pin, GPIO_HIGH);

        /* Configure Keys: Input, Enable Pull-up */
        hal_gpio_init_input(mappings[i].key_pin, GPIO_PULL_UP_ENABLE);
    }

    /* 2. Main Loop */
    while (1) {
        for (int i = 0; i < MAP_COUNT; i++) {
            /* 
             * Logic: 
             * - Key is active-low (Pressed = LOW)
             * - LED is active-low (On = LOW)
             * Direct mapping: LED state = Key state
             */
            hal_gpio_state_t key_state = hal_gpio_get(mappings[i].key_pin);
            hal_gpio_set(mappings[i].led_pin, key_state);
        }
    }

    return 0;
}
