#include "hal/hal_gpio.h"
#include "ll_gpio.h"

void hal_gpio_init_output(hal_gpio_pin_t pin) {
    ll_gpio_init_output(pin);
}

void hal_gpio_init_input(hal_gpio_pin_t pin, hal_gpio_pull_t pull) {
    ll_gpio_init_input(pin, (pull == GPIO_PULL_UP_ENABLE) ? 1 : 0);
}

void hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
    ll_gpio_set(pin, state);
}

hal_gpio_state_t hal_gpio_get(hal_gpio_pin_t pin) {
    return ll_gpio_get(pin);
}

void hal_gpio_toggle(hal_gpio_pin_t pin) {
    ll_gpio_toggle(pin);
}
