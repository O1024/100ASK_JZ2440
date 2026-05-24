/**
 * @file s3c2440_uart.c
 * @brief Professional S3C2440 UART Driver Implementation (FIFO Enabled)
 */

#include "hal/hal_uart.h"
#include "hal/hal_clock.h"
#include "hal/hal_timer.h"
#include "s3c2440_soc.h"

/* --- UART Register Bit Definitions --- */
#define UTRSTAT_TX_EMPTY    (1 << 2)
#define UTRSTAT_RX_READY    (1 << 0)
#define UFCON_FIFO_ENABLE   (1 << 0)
#define UFCON_TX_RESET      (1 << 2)
#define UFCON_RX_RESET      (1 << 1)

#define UFSTAT_TX_FULL      (1 << 14)
#define UFSTAT_RX_FULL      (1 << 6)
#define UFSTAT_RX_COUNT     (0x3F)

/**
 * @brief Initialize UART0 with 64-byte Hardware FIFO
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
    
    /* 4. Enable FIFO and reset it */
    UART0->UFCON = UFCON_FIFO_ENABLE | UFCON_TX_RESET | UFCON_RX_RESET;

    /* 5. Baud Rate Calculation:
     * UBRDIVn = (int)(PCLK / (baud * 16)) - 1
     */
    uint32_t pclk = hal_clock_get_pclk();
    UART0->UBRDIV = (pclk / (baud_rate * 16)) - 1;
}

/**
 * @brief Send a single character (Blocking)
 */
void hal_uart_putc(char c) {
    /* Wait until TX FIFO is not full */
    while (UART0->UFSTAT & UFSTAT_TX_FULL);
    
    /* Write data to transmit register */
    UART0->UTXH = (uint8_t)c;
}

/**
 * @brief Receive a single character (Blocking)
 */
char hal_uart_getc(void) {
    /* Wait until RX FIFO has data (Count > 0 OR Full bit is set) */
    while ((UART0->UFSTAT & (UFSTAT_RX_COUNT | UFSTAT_RX_FULL)) == 0);
    
    /* Read data from receive register */
    return (char)UART0->URXH;
}

/**
 * @brief Send a null-terminated string
 */
void hal_uart_puts(const char *str) {
    if (!str) return;
    
    while (*str) {
        if (*str == '\n') {
            hal_uart_putc('\r');
        }
        hal_uart_putc(*str++);
    }
}

int hal_uart_tstc(void) {
    return (UART0->UFSTAT & (UFSTAT_RX_COUNT | UFSTAT_RX_FULL)) ? 1 : 0;
}

int hal_uart_getc_timeout(uint32_t timeout_ms, char *c) {
    uint32_t target_ticks = timeout_ms * 31; /* Approximate to 31 ticks/ms */
    uint16_t start = hal_timer4_get_ticks();
    uint32_t elapsed = 0;
    
    /* Wait until RX FIFO has data (Count > 0 OR Full bit is set) */
    while ((UART0->UFSTAT & (UFSTAT_RX_COUNT | UFSTAT_RX_FULL)) == 0) {
        uint16_t current = hal_timer4_get_ticks();
        
        if (current <= start) {
            elapsed += (start - current);
        } else {
            elapsed += (start + (0xFFFF - current));
        }
        
        start = current;
        
        if (elapsed >= target_ticks) {
            return -1; /* Timeout */
        }
    }
    
    *c = (char)UART0->URXH;
    return 0;
}

void hal_uart_flush(void) {
    /* Reset RX FIFO directly */
    UART0->UFCON |= UFCON_RX_RESET;
}

void hal_uart_wait_tx_done(void) {
    /* Wait until TX FIFO count is 0 and shifter is empty */
    /* UTRSTAT_TX_EMPTY means BOTH FIFO and shifter are empty if FIFO is enabled */
    while (!(UART0->UTRSTAT & UTRSTAT_TX_EMPTY));
}
