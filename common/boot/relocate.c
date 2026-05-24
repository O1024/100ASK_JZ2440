/**
 * @file relocate.c
 * @brief Simple Runtime Environment Setup
 */

#include <stdint.h>

extern uint32_t _data_flash_start;
extern uint32_t _data_start;
extern uint32_t _data_end;
extern uint32_t _bss_start;
extern uint32_t _bss_end;

/**
 * @brief Initialize C data and clear BSS
 */
void hal_system_init(void) {
    uint32_t *src = &_data_flash_start;
    uint32_t *dest = &_data_start;
    uint32_t *end = &_data_end;

    /* Copy .data from Flash to RAM */
    while (dest < end) {
        *dest++ = *src++;
    }

    /* Zero .bss */
    dest = &_bss_start;
    end = &_bss_end;
    while (dest < end) {
        *dest++ = 0;
    }
}
