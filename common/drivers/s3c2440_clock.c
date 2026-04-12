/**
 * @file s3c2440_clock.c
 * @brief S3C2440 Clock System Implementation (Struct-based)
 */

#include "hal/hal_clock.h"
#include "s3c2440_soc.h"

void hal_clock_init(void) {
    CLK_PWR->LOCKTIME = 0xFFFFFFFF;
    CLK_PWR->CLKDIVN = 0x05;
    
    /* If CLKDIVN is not 0, CPU bus mode should be changed from fast bus mode to asynchronous bus mode */
    __asm__(
        "mrc p15, 0, r1, c1, c0, 0\n"
        "orr r1, r1, #0xc0000000\n"
        "mcr p15, 0, r1, c1, c0, 0\n"
    );
    
    /* FCLK = 400MHz, HCLK = 100MHz, PCLK = 50MHz */
    CLK_PWR->MPLLCON = (92 << 12) | (1 << 4) | (1);
}

void hal_clock_reset(void) {
    CLK_PWR->CLKDIVN = 0;
    
    __asm__(
        "mrc p15, 0, r1, c1, c0, 0\n"
        "bic r1, r1, #0xc0000000\n"
        "mcr p15, 0, r1, c1, c0, 0\n"
    );
    
    /* 12MHz xtal: MDIV=0x96, PDIV=5, SDIV=2 */
    CLK_PWR->MPLLCON = (0x96 << 12) | (0x05 << 4) | (0x02);
}

uint32_t hal_clock_get_pclk(void) {
    /* Since we only support fixed configurations for now, we return 50MHz or 12MHz based on MPLL */
    if (CLK_PWR->MPLLCON == ((92 << 12) | (1 << 4) | (1))) {
        return 50000000;
    } else {
        return 12000000;
    }
}
