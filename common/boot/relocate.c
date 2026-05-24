/**
 * @file relocate.c
 * @brief Simple Runtime Environment Setup
 */

#include <stdint.h>

/* Use __ prefix to match nor.lds */
extern uint32_t __data_flash_start;
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __bss_start;
extern uint32_t __bss_end;

/**
 * @brief Initialize C data and clear BSS
 */
void hal_system_init(void) {
    uint32_t *src = &__data_flash_start;
    uint32_t *dest = &__data_start;
    uint32_t *end = &__data_end;

    /* Copy .data from Flash to RAM */
    while (dest < end) {
        *dest++ = *src++;
    }

    /* Zero .bss */
    dest = &__bss_start;
    end = &__bss_end;
    while (dest < end) {
        *dest++ = 0;
    }
}
