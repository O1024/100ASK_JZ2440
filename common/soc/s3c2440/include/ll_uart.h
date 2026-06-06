#ifndef __LL_UART_H__
#define __LL_UART_H__

#include <stdint.h>

void ll_uart_init(uint32_t baud_rate);
void ll_uart_putc(char c);
char ll_uart_getc(void);
int  ll_uart_tstc(void);
void ll_uart_flush(void);
void ll_uart_wait_tx_done(void);

#endif /* __LL_UART_H__ */
