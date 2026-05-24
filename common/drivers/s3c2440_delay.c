/**
 * @file s3c2440_delay.c
 * @brief Simple software delay loop implementation
 */

#include "hal/hal_delay.h"

void hal_delay(volatile uint32_t count) {
    while (count--) {
        /* Busy wait */
    }
}
