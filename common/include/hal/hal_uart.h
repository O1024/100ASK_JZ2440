#ifndef __HAL_UART_H__
#define __HAL_UART_H__

#include <stdint.h>

/**
 * @brief 初始化 UART
 */
void hal_uart_init(uint32_t baud_rate);

/**
 * @brief 发送单个字节
 */
void hal_uart_putc(char c);

/**
 * @brief 接收单个字节 (阻塞)
 */
char hal_uart_getc(void);

/**
 * @brief 发送字符串
 */
void hal_uart_puts(const char *str);

/**
 * @brief Check if a character is available in the RX buffer
 * @return 1 if available, 0 if empty
 */
int hal_uart_tstc(void);

/**
 * @brief Send a character with timeout
 * @param timeout_ms Timeout in milliseconds
 * @param c Pointer to store the received character
 * @return 0 on success, -1 on timeout
 */
int hal_uart_getc_timeout(uint32_t timeout_ms, char *c);

/**
 * @brief Flush (clear) the UART receive buffer
 */
void hal_uart_flush(void);

/**
 * @brief Wait until all transmitted data has physically left the UART
 */
void hal_uart_wait_tx_done(void);

#endif // __HAL_UART_H__
