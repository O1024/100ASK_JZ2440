#include "s3c2440_soc.h"

void delay(volatile int d) {
  while (d--)
    ;
}

int sdram_test(void) {
  volatile unsigned int *p = (volatile unsigned int *)0x30000000;
  int i;

  // 写入测试数据
  for (i = 0; i < 1024; i++) {
    p[i] = 0x55AA55AA + i;
  }

  // 校验数据
  for (i = 0; i < 1024; i++) {
    if (p[i] != (0x55AA55AA + i)) {
      return -1; // 测试失败
    }
  }

  return 0; // 测试成功
}

int main(void) {
  /*
   * 配置 GPF4, GPF5, GPF6 为输出模式
   */
  GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
  GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));

  if (sdram_test() != 0) {
    // 测试失败：所有 LED 快速闪烁
    while (1) {
      GPFDAT &= ~((1 << 4) | (1 << 5) | (1 << 6));
      delay(100000);
      GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));
      delay(100000);
    }
  }

  // 测试成功：进入流水灯
  while (1) {
    // LED1 亮 (GPF4=0)
    GPFDAT = (GPFDAT | (7 << 4)) & ~(1 << 4);
    delay(500000);

    // LED2 亮 (GPF5=0)
    GPFDAT = (GPFDAT | (7 << 4)) & ~(1 << 5);
    delay(500000);

    // LED4 亮 (GPF6=0)
    GPFDAT = (GPFDAT | (7 << 4)) & ~(1 << 6);
    delay(500000);
  }

  return 0;
}