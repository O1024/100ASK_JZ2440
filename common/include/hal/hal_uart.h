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

#endif // __HAL_UART_H__
