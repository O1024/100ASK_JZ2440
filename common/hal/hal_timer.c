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
