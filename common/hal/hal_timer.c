#include "hal/hal_timer.h"
#include "ll_timer.h"

void hal_timer4_init(uint32_t ms) {
    ll_timer4_init(ms);
}

void hal_timer4_start(void) {
    ll_timer4_start();
}

void hal_timer4_stop(void) {
    ll_timer4_stop();
}

void hal_timer4_init_freerun(void) {
    ll_timer4_init_freerun();
}

uint16_t hal_timer4_get_ticks(void) {
    return ll_timer4_get_ticks();
}

void hal_timer4_set_handler(void (*handler)(void)) {
    ll_timer4_set_handler(handler);
}

void hal_timer4_reset_overflows(void) {
    ll_timer4_reset_overflows();
}

uint32_t hal_timer4_get_overflows(void) {
    return ll_timer4_get_overflows();
}

uint32_t hal_timer4_get_elapsed_ticks(uint16_t start_ticks, uint16_t end_ticks) {
    /* Timer4 is a down-counter: elapsed = start - end (uint16_t wrap-around) */
    uint32_t delta = (uint32_t)((uint16_t)(start_ticks - end_ticks));
    uint32_t overflows = ll_timer4_get_overflows();
    return delta + (overflows * 65536U);
}

uint32_t hal_timer4_ticks_to_us(uint32_t ticks) {
    /* PCLK = 50MHz, prescaler = 100, divider = 16
     * Timer frequency = 50MHz / 100 / 16 = 31250 Hz
     * 1 tick = 1 / 31250 s = 32 us
     */
    return (ticks * 32U);
}
