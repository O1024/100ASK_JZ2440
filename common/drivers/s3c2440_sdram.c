/**
 * @file s3c2440_sdram.c
 * @brief Professional S3C2440 SDRAM Controller Implementation
 */

#include "hal/hal_sdram.h"
#include "s3c2440_soc.h"

/**
 * @brief Initialize Memory Controller for SDRAM
 * 
 * Target: HY57V561620 (32MB x 2 = 64MB)
 * Bus Width: 32-bit (Bank 6)
 * CAS Latency: 3 cycles
 */
void hal_sdram_init(void) {
    /* 1. BWSCON: Bank 6 = 32-bit, Use nBE, nwait, Disable Addr-Data Mux */
    MEM_CTL->BWSCON = 0x22000000;

    /* 2. BANKCON0-5: Default values for SRAM/Flash banks */
    for (int i = 0; i <= 5; i++) {
        MEM_CTL->BANKCON[i] = 0x00000700;
    }

    /* 3. BANKCON6: SDRAM, Trp=20ns, Tsrc=20ns, CAS=3 cycles */
    MEM_CTL->BANKCON[6] = 0x00018001;
    MEM_CTL->BANKCON[7] = 0x00018001;

    /* 4. REFRESH: Enable, Auto-refresh, Trp=20ns, Tfrc=70ns
     * Refresh Count = 2^11 + 1 - PCLK * period / 10^6
     * For 100MHz HCLK: 0x008C0000 + 1268 = 0x008C04F3
     */
    MEM_CTL->REFRESH = 0x008C04F3;

    /* 5. BANKSIZE: 64MB, Burst-enable, SCLK power-save, MSS on */
    MEM_CTL->BANKSIZE = 0x000000B1;

    /* 6. MRSRB6/7: Mode Register Set (CL=3) */
    MEM_CTL->MRSRB6 = 0x00000030;
    MEM_CTL->MRSRB7 = 0x00000030;
}
