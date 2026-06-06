#include "hal/hal_sdram.h"
#include "ll_sdram.h"

void hal_sdram_init(void) {
    ll_sdram_init();
}
