/**
 * @file main.c
 * @brief SDRAM Diagnostic Tool (New Bootstrap Pattern)
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_sdram.h"
#include <stdint.h>

void hal_system_init(void);

int main(void) {
    /* 1. Critical Hardware Setup (Necessary if target RAM is SDRAM) */
    hal_clock_init();
    hal_sdram_init();

    /* 2. Initialize C Runtime (Copies data/bss to SDRAM if RAM_TARGET=sdram) */
    hal_system_init();

    /* --- Full SDRAM Environment Ready --- */
    hal_uart_init(115200);
    hal_uart_puts("\r\n--- JZ2440 SDRAM Boot Verification ---\r\n");
    
    /* Memory Test Logic... (Simplified for verification) */
    volatile uint32_t *p = (volatile uint32_t *)SDRAM_BASE;
    p[0] = 0x55AA55AA;
    if (p[0] == 0x55AA55AA) {
        hal_uart_puts("SDRAM Access OK!\r\n");
    }

    while (1);
    return 0;
}
