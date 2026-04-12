/**
 * @file main.c
 * @brief Professional Ethernet Loopback Test for JZ2440
 */

#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_eth.h"
#include "hal/hal_gpio.h"
#include <stdint.h>
#include <string.h>

#define HEARTBEAT_LED   GPF4

static void delay(volatile int count) {
    while (count--) __asm__("nop");
}

void run_ethernet_test(void) {
    const char *test_msg = "JZ2440 Unified SDK - Ethernet Loopback Test Pattern 1234567890";
    uint8_t tx_payload[128];
    uint8_t rx_buf[1536];
    uint32_t payload_len = strlen(test_msg) + 1;
    int rx_len;
    int retry = 0;

    memcpy(tx_payload, test_msg, payload_len);

    hal_uart_puts("\r\n[STEP 1] Initializing Ethernet...\r\n");
    hal_eth_init_loopback();
    hal_eth_dump_status();

    hal_uart_puts("[STEP 2] Sending Frame...\r\n");
    hal_eth_tx(tx_payload, payload_len);

    hal_uart_puts("[STEP 3] Receiving (5s Timeout)...\r\n");
    int timeout = 500000;
    while (timeout--) {
        rx_len = hal_eth_rx(rx_buf);
        if (rx_len < 0) {
            if (++retry > 3) break;
            hal_eth_tx(tx_payload, payload_len);
            continue;
        }
        
        if (rx_len > 0) {
            hal_uart_puts("\r\n[STEP 4] Analyzing Data...\r\n");
            hal_uart_puts("Received Length: ");
            int l = rx_len;
            hal_uart_putc('0' + (l/1000)%10);
            hal_uart_putc('0' + (l/100)%10);
            hal_uart_putc('0' + (l/10)%10);
            hal_uart_putc('0' + (l%10));
            hal_uart_puts(" bytes\r\n");

            hal_uart_puts("Payload: ");
            for (int i = 14; i < rx_len - 4; i++) {
                if (rx_buf[i] >= 32 && rx_buf[i] <= 126) hal_uart_putc(rx_buf[i]);
                else hal_uart_putc('.');
            }
            hal_uart_puts("\r\n");

            if (memcmp(tx_payload, &rx_buf[14], payload_len) == 0) {
                hal_uart_puts("\r\n>>> RESULT: VERIFICATION SUCCESS <<<\r\n");
            } else {
                hal_uart_puts("\r\n>>> RESULT: VERIFICATION FAILED (Data Mismatch) <<<\r\n");
            }
            return;
        }
        delay(10);
    }
    hal_uart_puts("\r\n>>> RESULT: FAILED (Timeout) <<<\r\n");
}

int main(void) {
    *(volatile uint32_t *)0x53000000 = 0; /* Stop WDT */
    hal_uart_init(115200);
    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("     JZ2440 ETHERNET DIAGNOSTIC TOOL    \r\n");
    hal_uart_puts("========================================\r\n");

    run_ethernet_test();

    while (1) {
        hal_gpio_toggle(HEARTBEAT_LED);
        delay(500000);
    }
    return 0;
}
