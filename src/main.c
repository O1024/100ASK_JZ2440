#include "s3c2440_soc.h"

void uart0_init(void) {
  /* 
   * 1. 配置引脚功能
   * GPH2 -> TXD0, GPH3 -> RXD0
   */
  GPHCON &= ~((3 << 4) | (3 << 6));
  GPHCON |= ((2 << 4) | (2 << 6));

  /* 2. 开启上拉电阻 */
  GPHUP &= ~((1 << 2) | (1 << 3));

  /* 3. 设置数据格式 (8N1) */
  ULCON0 = 0x3;

  /* 4. 设置工作模式 (轮询) */
  UCON0 = 0x5;

  /* 5. 禁用 FIFO 和流量控制 */
  UFCON0 = 0;
  UMCON0 = 0;

  /* 
   * 6. 设置波特率 (UBRDIV0)
   * 公式: UBRDIVn = (int)(PCLK / (bps * 16)) - 1
   * 现在的 PCLK = 50MHz
   * 目标波特率 = 115200: 50000000 / (115200 * 16) - 1 = 26.12 -> 取 26
   */
  UBRDIV0 = 26; 
}

void uart0_putchar(char c) {
  while (!(UTRSTAT0 & (1 << 2)));
  UTXH0 = c;
}

char uart0_getchar(void) {
  while (!(UTRSTAT0 & (1 << 0)));
  return (char)URXH0;
}

void uart0_puts(const char *s) {
  while (*s) {
    uart0_putchar(*s++);
  }
}

int main(void) {
  uart0_init();

  uart0_puts("\r\nUART0 Initialized (PCLK=50MHz, Baud=115200)\r\n");
  uart0_puts("Type something to echo:\r\n");

  while (1) {
    char c = uart0_getchar();
    if (c == '\r') {
      uart0_puts("\r\n");
    } else {
      uart0_putchar(c);
    }
  }

  return 0;
}