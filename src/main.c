#include "s3c2440_soc.h"

void uart0_init(void) {
  /* 
   * 1. 配置引脚功能
   * GPH2 -> TXD0, GPH3 -> RXD0
   * GPHCON [5:4] = 10, [7:6] = 10
   */
  GPHCON &= ~((3 << 4) | (3 << 6));
  GPHCON |= ((2 << 4) | (2 << 6));

  /* 2. 开启上拉电阻 */
  GPHUP &= ~((1 << 2) | (1 << 3));

  /* 
   * 3. 设置数据格式 (ULCON0)
   * [1:0] = 11 (8-bit data)
   * [2]   = 0  (1 stop bit)
   * [5:3] = 000 (No parity)
   * [6]   = 0  (Normal mode)
   */
  ULCON0 = 0x3;

  /* 
   * 4. 设置工作模式 (UCON0)
   * [3:0] = 0101 (Interrupt or Polling mode for Rx and Tx)
   * [11:10] = 00 (Clock source: PCLK)
   */
  UCON0 = 0x5;

  /* 5. 禁用 FIFO 和流量控制 (UFCON0, UMCON0) */
  UFCON0 = 0;
  UMCON0 = 0;

  /* 
   * 6. 设置波特率 (UBRDIV0)
   * 公式: UBRDIVn = (int)(PCLK / (bps * 16)) - 1
   * 目前 PCLK = 12MHz, 目标波特率 = 115200 (虽然误差大，先尝试)
   * 12000000 / (115200 * 16) - 1 = 5.51
   */
  UBRDIV0 = 5;
}

void uart0_putchar(char c) {
  /* 等待发送缓冲为空 (UTRSTAT0 [2]) */
  while (!(UTRSTAT0 & (1 << 2)));
  /* 发送数据 */
  UTXH0 = c;
}

char uart0_getchar(void) {
  /* 等待接收缓冲有数据 (UTRSTAT0 [0]) */
  while (!(UTRSTAT0 & (1 << 0)));
  /* 返回接收到的数据 */
  return (char)URXH0;
}

void uart0_puts(const char *s) {
  while (*s) {
    uart0_putchar(*s++);
  }
}

int main(void) {
  uart0_init();

  uart0_puts("\r\nUART0 Initialized (PCLK=12MHz, Baud=115200?)\r\n");
  uart0_puts("Type something to echo:\r\n");

  while (1) {
    char c = uart0_getchar();
    
    /* 处理回显 */
    if (c == '\r') {
      uart0_puts("\r\n");
    } else {
      uart0_putchar(c);
    }
  }

  return 0;
}
