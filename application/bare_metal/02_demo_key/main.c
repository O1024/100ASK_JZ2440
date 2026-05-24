/**
 * @file main.c
 * @brief Professional Key Control Application
 * 
 * This example demonstrates professional embedded C coding practices for input:
 * 1. Semantic array sizing (ARRAY_SIZE macro).
 * 2. Hardware mapping encapsulation via structs.
 * 3. Basic software debouncing for mechanical switches.
 * 4. Separation of initialization from the main polling loop.
 */

#include "hal/hal_gpio.h"
#include "hal/hal_delay.h"
#include <stdint.h>
#include <stddef.h>

/* Macro to safely calculate the number of elements in an array */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

/**
 * @brief Hardware mapping structure for Key-to-LED associations
 */
typedef struct {
    hal_gpio_pin_t key_pin;
    hal_gpio_pin_t led_pin;
} key_led_map_t;

/* Define the specific mapping for the JZ2440 board */
static const key_led_map_t mappings[] = {
    {GPG3, GPF4}, /* S4 -> LED1 */
    {GPF2, GPF5}, /* S3 -> LED2 */
    {GPF0, GPF6}, /* S2 -> LED3 */
};

/**
 * @brief Initialize hardware peripherals
 * 
 * Configures the LED pins as outputs (default off) and 
 * Key pins as inputs with internal pull-ups enabled.
 */
static void hw_init(void) {
    for (size_t i = 0; i < ARRAY_SIZE(mappings); i++) {
        /* LEDs: Output, Active Low (High = Off) */
        hal_gpio_init_output(mappings[i].led_pin);
        hal_gpio_set(mappings[i].led_pin, GPIO_HIGH);
        
        /* Keys: Input, Active Low (Pull-up enabled) */
        hal_gpio_init_input(mappings[i].key_pin, GPIO_PULL_UP_ENABLE);
    }
}

/**
 * @brief Main polling loop with basic software debouncing
 * 
 * Continuously scans the keys. If a key press is detected, it waits
 * for a short debounce period and checks again to confirm before
 * updating the LED state.
 */
static void key_scan_loop(void) {
    while (1) {
        for (size_t i = 0; i < ARRAY_SIZE(mappings); i++) {
            hal_gpio_state_t state1 = hal_gpio_get(mappings[i].key_pin);
            
            /* Assuming active low: GPIO_LOW means pressed */
            if (state1 == GPIO_LOW) {
                /* Basic software debounce: wait a short period and read again */
                hal_delay(1000); 
                hal_gpio_state_t state2 = hal_gpio_get(mappings[i].key_pin);
                
                if (state2 == GPIO_LOW) {
                    /* Confirmed press, turn LED ON */
                    hal_gpio_set(mappings[i].led_pin, GPIO_LOW);
                }
            } else {
                /* Key released, turn LED OFF */
                hal_gpio_set(mappings[i].led_pin, GPIO_HIGH);
            }
        }
    }
}

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 1. Initialize C Runtime (Relocate data, clear BSS) */
    hal_system_init();

    /* 2. Hardware Initialization */
    hw_init();

    /* 3. Enter Main Business Logic Loop */
    key_scan_loop();

    return 0;
}
