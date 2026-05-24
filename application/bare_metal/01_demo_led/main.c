/**
 * @file main.c
 * @brief Professional LED Chaser Application
 * 
 * This example demonstrates professional embedded C coding practices:
 * 1. Semantic array sizing (ARRAY_SIZE macro).
 * 2. Separation of hardware initialization from business logic.
 * 3. Clear Doxygen-style documentation.
 */

#include "hal/hal_gpio.h"
#include "hal/hal_delay.h"
#include <stdint.h>
#include <stddef.h>

/* Macro to safely calculate the number of elements in an array */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

/* Hardware mapping for LEDs */
static const hal_gpio_pin_t LED_PINS[] = {GPF4, GPF5, GPF6};

/**
 * @brief Initialize all GPIO pins connected to LEDs as outputs.
 * 
 * This function iterates through the configured LED pin array
 * and sets their direction to output. It should be called once
 * during system startup.
 */
static void led_init(void) {
    for (size_t i = 0; i < ARRAY_SIZE(LED_PINS); i++) {
        hal_gpio_init_output(LED_PINS[i]);
        /* Turn off all LEDs initially (assuming active low, HIGH = off) */
        hal_gpio_set(LED_PINS[i], GPIO_HIGH);
    }
}

/**
 * @brief Execute the LED chaser visual effect.
 * 
 * This function turns on each LED in sequence, waits for a
 * predefined delay, and then turns it off before moving to the
 * next one. It loops indefinitely.
 */
static void led_chaser_loop(void) {
    while (1) {
        for (size_t i = 0; i < ARRAY_SIZE(LED_PINS); i++) {
            /* Turn LED ON (Active Low) */
            hal_gpio_set(LED_PINS[i], GPIO_LOW);
            
            /* Wait to keep the LED visible */
            hal_delay(100000);
            
            /* Turn LED OFF */
            hal_gpio_set(LED_PINS[i], GPIO_HIGH);
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
    led_init();

    /* 3. Enter Main Business Logic Loop */
    led_chaser_loop();

    return 0;
}
