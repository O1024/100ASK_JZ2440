/**
 * @file s3c2440_delay.c
 * @brief Simple software delay loop implementation
 */

#include "ll_delay.h"

void ll_delay(volatile uint32_t count) {
    while (count--) {
        /* Busy wait */
    }
}
