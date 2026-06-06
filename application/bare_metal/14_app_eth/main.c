/**
 * @file main.c
 * @brief Ethernet (DM9000) Test using BSP + HAL
 */

#include "bsp_init.h"
#include "hal/hal_eth.h"
#include "hal/hal_uart.h"
#include <stdint.h>
#include <string.h>

extern void hal_system_init(void);

static uint8_t tx_payload[64];
static uint8_t rx_buffer[1536];

int main(void) {
#ifdef TARGET_SDRAM
    bsp_clock_init();
    bsp_sdram_init();
#endif

    hal_system_init();

#ifndef TARGET_SDRAM
    bsp_clock_init();
#endif

    bsp_uart_init();

    BSP_PRINT_BANNER("14 Ethernet (DM9000) Test");
    hal_uart_puts("Mode: MAC Loopback\r\n");

    bsp_eth_init();
    hal_uart_puts("[ETH] DM9000 loopback initialized.\r\n");

    const char *test_msg = "JZ2440 Ping!";
    uint32_t payload_len = strlen(test_msg) + 1;
    memcpy(tx_payload, test_msg, payload_len);

    hal_uart_puts("[ETH] Sending packet... ");
    hal_eth_tx(tx_payload, payload_len);
    hal_uart_puts("Done\r\n");

    while (1) {
        int len = hal_eth_rx(rx_buffer);
        if (len > 0) {
            hal_uart_puts("[ETH] Packet Received! Length: ");
            hal_uart_putc('0' + (len / 100) % 10);
            hal_uart_putc('0' + (len / 10) % 10);
            hal_uart_putc('0' + len % 10);
            hal_uart_puts("\r\n");
        }
    }

    return 0;
}
