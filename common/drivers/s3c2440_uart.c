/**
 * @file s3c2440_uart.c
 * @brief Professional S3C2440 UART Driver Implementation
 */

#include "hal/hal_uart.h"
#include "hal/hal_clock.h"
#include "s3c2440_soc.h"

/* --- UART Register Bit Definitions --- */
#define UTRSTAT_TX_EMPTY    (1 << 2)
#define UTRSTAT_RX_READY    (1 << 0)

/**
 * @brief Initialize UART0
 * @param baud_rate Target baud rate (e.g., 115200)
 */
void hal_uart_init(uint32_t baud_rate) {
    /* 1. Configure GPIO: GPH2 -> TXD0, GPH3 -> RXD0 */
    gpio_port_t *port_h = GPIO_PORT(PORT_H);
    port_h->CON &= ~((3 << 4) | (3 << 6));
    port_h->CON |=  ((2 << 4) | (2 << 6));
    
    /* Enable internal pull-up for RX/TX lines */
    port_h->UP &= ~((1 << 2) | (1 << 3));

    /* 2. Format: 8 Bits, No Parity, 1 Stop Bit (8N1) */
    UART0->ULCON = 0x03;

    /* 3. Control: Polling Mode, Disable Interrupts, No DMA */
    UART0->UCON = 0x05; 

    /* 4. Baud Rate Calculation:
     * UBRDIVn = (int)(PCLK / (baud * 16)) - 1
     */
    uint32_t pclk = hal_clock_get_pclk();
    UART0->UBRDIV = (pclk / (baud_rate * 16)) - 1;
}

/**
 * @brief Send a single character (Blocking)
 */
void hal_uart_putc(char c) {
    /* Wait until Transmit buffer is empty */
    while (!(UART0->UTRSTAT & UTRSTAT_TX_EMPTY));
    
    /* Write data to transmit register */
    UART0->UTXH = (uint8_t)c;
}

/**
 * @brief Receive a single character (Blocking)
 */
char hal_uart_getc(void) {
    /* Wait until Receive buffer has data */
    while (!(UART0->UTRSTAT & UTRSTAT_RX_READY));
    
    /* Read data from receive register */
    return (char)UART0->URXH;
}

/**
 * @brief Send a null-terminated string
 */
void hal_uart_puts(const char *str) {
    if (!str) return;
    
    while (*str) {
        /* Standardize line endings to \r\n */
        if (*str == '\n') {
            hal_uart_putc('\r');
        }
        hal_uart_putc(*str++);
    }
}
