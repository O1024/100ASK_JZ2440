#include "s3c2440_soc.h"

void delay(volatile int d) {
  while (d--)
    ;
}

int main(void) {
  /*
   * 配置 GPF4, GPF5, GPF6 为输出模式
   * GPFCON [9:8]   = 01 (GPF4 output)
   * GPFCON [11:10] = 01 (GPF5 output)
   * GPFCON [13:12] = 01 (GPF6 output)
   */
  GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
  GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));

  while (1) {
    // LED1 亮 (GPF4=0), 其他灭
    GPFDAT = ~(1 << 4);
    delay(500000);

    // LED2 亮 (GPF5=0), 其他灭
    GPFDAT = ~(1 << 5);
    delay(500000);

    // LED4 亮 (GPF6=0), 其他灭
    GPFDAT = ~(1 << 6);
    delay(500000);
  }

  return 0;
}