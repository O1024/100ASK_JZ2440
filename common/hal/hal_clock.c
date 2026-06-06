#include "hal/hal_clock.h"
#include "ll_clock.h"

void hal_clock_init(void) {
    ll_clock_init();
}

void hal_clock_reset(void) {
    ll_clock_reset();
}

uint32_t hal_clock_get_fclk(void) {
    return ll_clock_get_fclk();
}

uint32_t hal_clock_get_hclk(void) {
    return ll_clock_get_hclk();
}

uint32_t hal_clock_get_pclk(void) {
    return ll_clock_get_pclk();
}
