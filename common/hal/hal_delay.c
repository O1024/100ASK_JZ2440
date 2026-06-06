#include "hal/hal_delay.h"
#include "ll_delay.h"

void hal_delay(uint32_t count) {
    ll_delay(count);
}
