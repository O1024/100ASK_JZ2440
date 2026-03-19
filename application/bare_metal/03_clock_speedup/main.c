/**
 * @file main.c
 * @brief Clock Speedup Experiment for JZ2440
 * 
 * Demonstrates the impact of raising the FCLK to 400MHz on execution speed.
 * Professional version: Reusable LED logic with clock initialization.
 */

#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"

/* --- Configuration --- */
static const hal_gpio_pin_t leds[] = {GPF4, GPF5, GPF6};
#define LED_COUNT (sizeof(leds) / sizeof(leds[0]))

/* Note: At 400MHz FCLK, 250000 iterations will be much faster than at default 12MHz */
#define DELAY_TICKS 250000

/**
 * @brief Software delay with NOP to prevent optimization
 */
static void delay(volatile int count) {
    while (count--) {
        __asm__("nop");
    }
}

/**
 * @brief Simple LED Chaser Step
 */
static void led_step(int index) {
    for (int i = 0; i < LED_COUNT; i++) {
        hal_gpio_set(leds[i], (i == index) ? GPIO_LOW : GPIO_HIGH);
    }
}

int main(void) {
    /* 1. Clock Initialization: Boost FCLK to 400MHz, HCLK to 100MHz, PCLK to 50MHz */
    hal_clock_init();

    /* 2. Hardware Initialization */
    for (int i = 0; i < LED_COUNT; i++) {
        hal_gpio_init_output(leds[i]);
        hal_gpio_set(leds[i], GPIO_HIGH); /* Start OFF */
    }

    int current_led = 0;

    /* 3. Main Loop: Visual speed comparison */
    while (1) {
        led_step(current_led);
        
        delay(DELAY_TICKS);

        if (++current_led >= LED_COUNT) {
            current_led = 0;
        }
    }

    return 0;
}
