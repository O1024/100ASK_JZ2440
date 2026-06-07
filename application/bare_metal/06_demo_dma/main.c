/**
 * @file main.c
 * @brief DMA vs CPU Performance Demo using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_uart.h"
#include "hal/hal_timer.h"
#include "hal/hal_dma.h"
#include <stdint.h>
#include <stddef.h>

extern void hal_system_init(void);

#define TEST_SIZE   (1024 * 1024)
#define SRC_ADDR    0x30100000
#define DST_ADDR    0x30200000

static void print_hex32(uint32_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    hal_uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        hal_uart_putc(hex_chars[(val >> i) & 0xF]);
    }
}

static void print_dec(uint32_t val) {
    char buf[12];
    int i = 11;
    buf[i] = '\0';
    if (val == 0) {
        buf[--i] = '0';
    } else {
        while (val > 0 && i > 0) {
            buf[--i] = (val % 10) + '0';
            val /= 10;
        }
    }
    hal_uart_puts(&buf[i]);
}

int main(void) {
    /* 1. Critical Early Initialization */
    bsp_clock_init();
    bsp_sdram_init();
    bsp_uart_init();
    hal_system_init();

    BSP_PRINT_BANNER("06 DMA vs CPU Performance Demo");
    hal_uart_puts("Transfer Size: 1 MB\r\n");
    hal_uart_puts("Source Addr  : "); print_hex32(SRC_ADDR); hal_uart_puts("\r\n");
    hal_uart_puts("Dest Addr    : "); print_hex32(DST_ADDR); hal_uart_puts("\r\n");

    hal_timer4_init_freerun();
    hal_timer4_start();

    volatile uint32_t *src = (volatile uint32_t *)SRC_ADDR;
    volatile uint32_t *dst = (volatile uint32_t *)DST_ADDR;
    uint32_t count = TEST_SIZE / 4;

    /* --- CPU Copy Test --- */
    hal_uart_puts("Preparing CPU Copy... ");
    for (uint32_t i = 0; i < count; i++) src[i] = i;
    for (uint32_t i = 0; i < count; i++) dst[i] = 0;
    hal_uart_puts("Done.\r\n");

    hal_uart_puts("Starting CPU Copy... ");
    hal_timer4_reset_overflows();
    uint16_t start_ticks = hal_timer4_get_ticks();
    for (uint32_t i = 0; i < count; i++) {
        dst[i] = src[i];
    }
    uint16_t end_ticks = hal_timer4_get_ticks();
    uint32_t cpu_ticks = hal_timer4_get_elapsed_ticks(start_ticks, end_ticks);
    hal_uart_puts("Done.\r\n");
    hal_uart_puts("CPU Ticks: "); print_dec(cpu_ticks); hal_uart_puts("\r\n");
    if (cpu_ticks > 0) {
        hal_uart_puts("CPU Speed: "); print_dec(262144 / cpu_ticks); hal_uart_puts(" Mbps\r\n");
    }

    int err = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (dst[i] != src[i]) { err = 1; break; }
    }
    if (err) hal_uart_puts("RESULT: CPU Copy FAILED!\r\n");
    else     hal_uart_puts("RESULT: CPU Copy PASSED.\r\n");

    hal_uart_puts("----------------------------------------\r\n");

    /* --- DMA Copy Test --- */
    hal_uart_puts("Preparing DMA Copy... ");
    for (uint32_t i = 0; i < count; i++) dst[i] = 0;
    hal_uart_puts("Done.\r\n");

    hal_dma_config_t dma_cfg;
    dma_cfg.src_addr       = SRC_ADDR;
    dma_cfg.src_bus        = DMA_BUS_AHB;
    dma_cfg.src_addr_mode  = DMA_ADDR_INC;
    dma_cfg.dst_addr       = DST_ADDR;
    dma_cfg.dst_bus        = DMA_BUS_AHB;
    dma_cfg.dst_addr_mode  = DMA_ADDR_INC;
    dma_cfg.data_size      = DMA_DATA_WORD;
    dma_cfg.trans_type     = DMA_TRANS_BURST4;
    dma_cfg.transfer_count = count;

    hal_dma_config_software(DMA_CH0, &dma_cfg);

    hal_uart_puts("Starting DMA Copy... ");
    hal_timer4_reset_overflows();
    start_ticks = hal_timer4_get_ticks();
    hal_dma_start(DMA_CH0);
    while (hal_dma_is_busy(DMA_CH0));
    end_ticks = hal_timer4_get_ticks();
    uint32_t dma_ticks = hal_timer4_get_elapsed_ticks(start_ticks, end_ticks);
    hal_uart_puts("Done.\r\n");
    hal_uart_puts("DMA Ticks: "); print_dec(dma_ticks); hal_uart_puts("\r\n");
    if (dma_ticks > 0) {
        hal_uart_puts("DMA Speed: "); print_dec(262144 / dma_ticks); hal_uart_puts(" Mbps\r\n");
    }

    err = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (dst[i] != src[i]) { err = 1; break; }
    }
    if (err) hal_uart_puts("RESULT: DMA Copy FAILED!\r\n");
    else     hal_uart_puts("RESULT: DMA Copy PASSED.\r\n");

    hal_uart_puts("----------------------------------------\r\n");
    if (dma_ticks > 0) {
        hal_uart_puts("DMA is approx ");
        print_dec(cpu_ticks / dma_ticks);
        hal_uart_puts(" times faster than CPU loop.\r\n");
    }
    hal_uart_puts("========================================\r\n");

    while (1);
    return 0;
}
