/**
 * @file main.c
 * @brief High-Performance Clock Configuration Example
 * 
 * This example demonstrates:
 * 1. Boot-time PLL configuration via hal_clock_init().
 *    (FCLK=400MHz for CPU, HCLK=100MHz for AHB, PCLK=50MHz for APB).
 * 2. Visual confirmation of clock speed increase. Since hal_delay()
 *    is a simple software loop, running the CPU at 400MHz instead of 
 *    the default 12MHz crystal frequency makes the LEDs blink 
 *    significantly faster.
 */

#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"
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
 * @brief Initialize hardware peripherals
 */
static void hw_init(void) {
    for (size_t i = 0; i < ARRAY_SIZE(LED_PINS); i++) {
        hal_gpio_init_output(LED_PINS[i]);
        /* Turn off all LEDs initially (active low) */
        hal_gpio_set(LED_PINS[i], GPIO_HIGH);
    }
}

/**
 * @brief Execute a high-speed LED chaser to demonstrate clock impact
 */
static void led_speed_test_loop(void) {
    while (1) {
        for (size_t i = 0; i < ARRAY_SIZE(LED_PINS); i++) {
            /* Turn LED ON */
            hal_gpio_set(LED_PINS[i], GPIO_LOW);
            
            /* 
             * Because FCLK is now 400MHz, this software delay 
             * will execute approximately 33 times faster than at 12MHz. 
             */
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
    /* 1. Critical Hardware Setup: Boost Clock BEFORE data relocation */
    hal_clock_init();

    /* 2. Initialize C Runtime (Relocate data, clear BSS) */
    hal_system_init();

    /* 3. Hardware Initialization */
    hw_init();

    /* 4. Enter test loop */
    led_speed_test_loop();

    return 0;
}
