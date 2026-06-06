/**
 * @file s3c2440_gpio.c
 * @brief Table-driven GPIO Driver Implementation (Struct-based)
 */

#include "ll_gpio.h"
#include "s3c2440_soc.h"

void ll_gpio_init_output(uint32_t pin) {
    uint32_t port_idx = LL_GPIO_PORT_GET(pin);
    uint32_t bit      = LL_GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    /* 1. Set data to HIGH first (Active Low LEDs Off) */
    port->DAT |= (1 << bit);

    /* 2. Configure as Output (01b) */
    port->CON &= ~(3 << (bit * 2));
    port->CON |=  (1 << (bit * 2));
}

void ll_gpio_init_input(uint32_t pin, int pull_up) {
    uint32_t port_idx = LL_GPIO_PORT_GET(pin);
    uint32_t bit      = LL_GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    /* 1. Configure as Input (00b) */
    port->CON &= ~(3 << (bit * 2));

    /* 2. Configure Pull-up (pull_up=1: enable, pull_up=0: disable) */
    if (pull_up) {
        port->UP &= ~(1 << bit);
    } else {
        port->UP |=  (1 << bit);
    }
}

/**
 * @brief Set the output state of a GPIO pin.
 */
void ll_gpio_set(uint32_t pin, uint32_t state) {
    uint32_t port_idx = LL_GPIO_PORT_GET(pin);
    uint32_t bit      = LL_GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    if (state) {
        port->DAT |=  (1 << bit);
    } else {
        port->DAT &= ~(1 << bit);
    }
}

uint32_t ll_gpio_get(uint32_t pin) {
    uint32_t port_idx = LL_GPIO_PORT_GET(pin);
    uint32_t bit      = LL_GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    return (port->DAT & (1 << bit)) ? 1 : 0;
}

/**
 * @brief Toggle the output state of a GPIO pin.
 */
void ll_gpio_toggle(uint32_t pin) {
    uint32_t port_idx = LL_GPIO_PORT_GET(pin);
    uint32_t bit      = LL_GPIO_PIN_GET(pin);
    gpio_port_t *port = GPIO_PORT(port_idx);

    port->DAT ^= (1 << bit);
}
