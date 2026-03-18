/**
 * @file hal_gpio.h
 * @brief Professional GPIO HAL Interface for JZ2440
 */

#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#include <stdint.h>

/**
 * @brief GPIO Pin Encoding: [15:8] Port Index, [7:0] Pin Index
 */
#define GPIO_PIN(port, pin)   (((port) << 8) | (pin))
#define GPIO_PORT_GET(pin)    (((pin) >> 8) & 0xFF)
#define GPIO_PIN_GET(pin)     ((pin) & 0xFF)

typedef enum {
    /* Port F: 0-7 */
    GPF0 = GPIO_PIN(5, 0), GPF1 = GPIO_PIN(5, 1), GPF2 = GPIO_PIN(5, 2), GPF3 = GPIO_PIN(5, 3),
    GPF4 = GPIO_PIN(5, 4), GPF5 = GPIO_PIN(5, 5), GPF6 = GPIO_PIN(5, 6), GPF7 = GPIO_PIN(5, 7),

    /* Port G: 0-15 */
    GPG0 = GPIO_PIN(6, 0), GPG3 = GPIO_PIN(6, 3), GPG11 = GPIO_PIN(6, 11),

    /* Port H: 0-10 */
    GPH0 = GPIO_PIN(7, 0), GPH2 = GPIO_PIN(7, 2), GPH3 = GPIO_PIN(7, 3),
} hal_gpio_pin_t;

typedef enum {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1
} hal_gpio_state_t;

typedef enum {
    GPIO_PULL_UP_ENABLE  = 0,
    GPIO_PULL_UP_DISABLE = 1
} hal_gpio_pull_t;

/* --- Core API --- */
void hal_gpio_init_output(hal_gpio_pin_t pin);
void hal_gpio_init_input(hal_gpio_pin_t pin, hal_gpio_pull_t pull);
void hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state);
hal_gpio_state_t hal_gpio_get(hal_gpio_pin_t pin);
void hal_gpio_toggle(hal_gpio_pin_t pin);

#endif /* __HAL_GPIO_H__ */
