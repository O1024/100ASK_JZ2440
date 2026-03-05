/**
 * @file s3c2440_clock.c
 * @brief S3C2440 Clock System Implementation
 */

#include "hal/hal_clock.h"
#include "s3c2440_soc.h"

void hal_clock_init(void) {
    LOCKTIME = 0xFFFFFFFF;
    CLKDIVN = 0x05;
    __asm__(
        "mrc p15, 0, r1, c1, c0, 0\n"
        "orr r1, r1, #0xc0000000\n"
        "mcr p15, 0, r1, c1, c0, 0\n"
    );
    MPLLCON = (92 << 12) | (1 << 4) | (1);
}

void hal_clock_reset(void) {
    /* 默认 12MHz 配置 (或者接近 12MHz) */
    CLKDIVN = 0;
    __asm__(
        "mrc p15, 0, r1, c1, c0, 0\n"
        "bic r1, r1, #0xc0000000\n"
        "mcr p15, 0, r1, c1, c0, 0\n"
    );
    /* 12MHz xtal: MDIV=0x96, PDIV=5, SDIV=2 得到约 12.06MHz */
    MPLLCON = (0x96 << 12) | (0x05 << 4) | (0x02);
}
