#ifndef __LL_GPIO_H__
#define __LL_GPIO_H__

#include <stdint.h>

#define LL_GPIO_PIN(port, pin)   (((port) << 8) | (pin))
#define LL_GPIO_PORT_GET(pin)    (((pin) >> 8) & 0xFF)
#define LL_GPIO_PIN_GET(pin)     ((pin) & 0xFF)

void ll_gpio_init_output(uint32_t pin);
void ll_gpio_init_input(uint32_t pin, int pull_up);
void ll_gpio_set(uint32_t pin, uint32_t state);
uint32_t ll_gpio_get(uint32_t pin);
void ll_gpio_toggle(uint32_t pin);

#endif /* __LL_GPIO_H__ */
