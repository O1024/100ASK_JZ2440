/**
 * @file main.c
 * @brief Ethernet (DM9000) Basic Test (Corrected API)
 */

#include "hal/hal_eth.h"
#include "hal/hal_uart.h"
#include "hal/hal_clock.h"
#include "hal/hal_sdram.h"
#include <stdint.h>
#include <string.h>

void hal_system_init(void);

static uint8_t tx_payload[64];
static uint8_t rx_buffer[1536];

int main(void) {
    /* 1. Low-level Hardware */
#ifdef TARGET_SDRAM
    hal_clock_init();
    hal_sdram_init();
#endif

    /* 2. Data Relocation */
    hal_system_init();

    /* 3. Peripheral Initialization */
#ifndef TARGET_SDRAM
    hal_clock_init();
#endif
    hal_uart_init(115200);
    hal_uart_puts("\r\n--- JZ2440 DM9000 Ethernet Test ---\r\n");

    /* Initialize in loopback mode */
    hal_eth_init_loopback();
    hal_uart_puts("[ETH] DM9000 loopback initialized.\r\n");

    /* Send test packet */
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
