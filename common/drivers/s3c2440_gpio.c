/**
 * @file s3c2440_gpio.c
 * @brief Table-driven GPIO Driver Implementation (Struct-based)
 */

#include "hal/hal_gpio.h"
#include "s3c2440_soc.h"

void hal_gpio_init_output(hal_gpio_pin_t pin) {
    uint32_t port_idx = GPIO_PORT_GET(pin);
    uint32_t bit      = GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    /* 1. Set data to HIGH first (Active Low LEDs Off) */
    port->DAT |= (1 << bit);

    /* 2. Configure as Output (01b) */
    port->CON &= ~(3 << (bit * 2));
    port->CON |=  (1 << (bit * 2));
}

void hal_gpio_init_input(hal_gpio_pin_t pin, hal_gpio_pull_t pull) {
    uint32_t port_idx = GPIO_PORT_GET(pin);
    uint32_t bit      = GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    /* 1. Configure as Input (00b) */
    port->CON &= ~(3 << (bit * 2));

    /* 2. Configure Pull-up */
    if (pull == GPIO_PULL_UP_ENABLE) {
        port->UP &= ~(1 << bit);
    } else {
        port->UP |=  (1 << bit);
    }
}

void hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
    uint32_t port_idx = GPIO_PORT_GET(pin);
    uint32_t bit      = GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    if (state == GPIO_HIGH) {
        port->DAT |=  (1 << bit);
    } else {
        port->DAT &= ~(1 << bit);
    }
}

hal_gpio_state_t hal_gpio_get(hal_gpio_pin_t pin) {
    uint32_t port_idx = GPIO_PORT_GET(pin);
    uint32_t bit      = GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    return (port->DAT & (1 << bit)) ? GPIO_HIGH : GPIO_LOW;
}

void hal_gpio_toggle(hal_gpio_pin_t pin) {
    uint32_t port_idx = GPIO_PORT_GET(pin);
    uint32_t bit      = GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    port->DAT ^= (1 << bit);
}
