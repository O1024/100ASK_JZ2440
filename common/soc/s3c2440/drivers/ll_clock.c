/**
 * @file s3c2440_clock.c
 * @brief S3C2440 Clock System Implementation (Struct-based)
 */

#include "ll_clock.h"
#include "s3c2440_soc.h"

void ll_clock_init(void) {
    CLK_PWR->LOCKTIME = 0xFFFFFFFF;
    CLK_PWR->CLKDIVN = 0x05;

    /* If CLKDIVN is not 0, CPU bus mode should be changed from fast bus mode to asynchronous bus
     * mode */
    __asm__("mrc p15, 0, r1, c1, c0, 0\n"
            "orr r1, r1, #0xc0000000\n"
            "mcr p15, 0, r1, c1, c0, 0\n");

    /* FCLK = 400MHz, HCLK = 100MHz, PCLK = 50MHz */
    CLK_PWR->MPLLCON = (92 << 12) | (1 << 4) | (1);

    /* Enable Peripheral Clocks in CLKCON
     * We keep WDT clock (bit 13) ON so we can explicitly disable it.
     */
    CLK_PWR->CLKCON |= (1 << 8) | (1 << 10) | (1 << 13);
}

void ll_clock_reset(void) {
    CLK_PWR->CLKDIVN = 0;

    __asm__("mrc p15, 0, r1, c1, c0, 0\n"
            "bic r1, r1, #0xc0000000\n"
            "mcr p15, 0, r1, c1, c0, 0\n");

    /* 12MHz xtal: MDIV=0x96, PDIV=5, SDIV=2 */
    CLK_PWR->MPLLCON = (0x96 << 12) | (0x05 << 4) | (0x02);
}

uint32_t ll_clock_get_fclk(void) {
    /* CLKDIVN=0x05: FCLK = 400MHz when MPLL is configured for 400MHz, else 12MHz */
    if (CLK_PWR->MPLLCON == ((92 << 12) | (1 << 4) | (1))) {
        return 400000000;
    } else {
        return 12000000;
    }
}

uint32_t ll_clock_get_hclk(void) {
    /* CLKDIVN=0x05: HCLK = FCLK / 4 */
    return ll_clock_get_fclk() / 4;
}

uint32_t ll_clock_get_pclk(void) {
    /* CLKDIVN=0x05: PCLK = HCLK / 2 = FCLK / 8 */
    return ll_clock_get_fclk() / 8;
}
