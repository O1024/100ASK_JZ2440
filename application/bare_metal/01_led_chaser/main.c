/**
 * @file main.c
 * @brief LED Chaser Application for JZ2440
 * 
 * Demonstrates basic GPIO control using the Unified HAL.
 * Professional version: Data-driven approach for LED management.
 */

#include "hal/hal_gpio.h"

/* --- Configuration --- */
static const hal_gpio_pin_t leds[] = {GPF4, GPF5, GPF6};
#define LED_COUNT (sizeof(leds) / sizeof(leds[0]))
#define DELAY_TICKS 100000

/**
 * @brief Simple software delay
 * @param count Number of iterations
 */
static void delay(volatile int count) {
    while (count--) {
        __asm__("nop"); /* Prevent compiler from optimizing away the loop */
    }
}

/**
 * @brief Updates LED states to create a "chaser" effect
 * @param active_index The index of the LED to be turned ON
 */
static void update_led_chaser(int active_index) {
    for (int i = 0; i < LED_COUNT; i++) {
        /* JZ2440 LEDs are active-low (LOW = ON, HIGH = OFF) */
        hal_gpio_set(leds[i], (i == active_index) ? GPIO_LOW : GPIO_HIGH);
    }
}

int main(void) {
    /* 1. Hardware Initialization */
    for (int i = 0; i < LED_COUNT; i++) {
        hal_gpio_init_output(leds[i]);
        hal_gpio_set(leds[i], GPIO_HIGH); /* Ensure all LEDs start OFF */
    }

    int current_led = 0;

    /* 2. Main Loop */
    while (1) {
        update_led_chaser(current_led);
        
        delay(DELAY_TICKS);

        /* Increment and wrap around index */
        if (++current_led >= LED_COUNT) {
            current_led = 0;
        }
    }

    return 0;
}
