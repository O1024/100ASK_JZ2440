/**
 * @file s3c2440_uart.c
 * @brief S3C2440 UART Driver Implementation (Struct-based)
 */

#include "hal/hal_uart.h"
#include "s3c2440_soc.h"

void hal_uart_init(uint32_t baud_rate) {
    /* 1. Configure GPH2 -> TXD0, GPH3 -> RXD0 */
    gpio_port_t *port_h = GPIO_PORT(PORT_H);
    port_h->CON &= ~((3 << 4) | (3 << 6));
    port_h->CON |=  ((2 << 4) | (2 << 6));
    
    /* Enable internal pull-up */
    port_h->UP &= ~((1 << 2) | (1 << 3));

    /* 2. Format: 8N1 */
    UART0->ULCON = 0x03;

    /* 3. Mode: Polling */
    UART0->UCON = 0x05; 

    /* 4. Baud Rate: UBRDIVn = (int)(PCLK / (bps * 16)) - 1 */
    if (baud_rate == 115200) {
        UART0->UBRDIV = 26;
    } else {
        UART0->UBRDIV = 26; 
    }
}

void hal_uart_putc(char c) {
    while (!(UART0->UTRSTAT & (1 << 2)));
    UART0->UTXH = (unsigned char)c;
}

char hal_uart_getc(void) {
    while (!(UART0->UTRSTAT & (1 << 0)));
    return UART0->URXH;
}

void hal_uart_puts(const char *str) {
    while (*str) {
        if (*str == '\n') {
            hal_uart_putc('\r');
        }
        hal_uart_putc(*str++);
    }
}
