#include "s3c2440_soc.h"

// 验证 .bss 段 (初始值为 0)
int g_i = 0;
// 验证 .data 段 (初始值为非 0)
int g_j = 10;

void delay(volatile int d) {
  while (d--)
    ;
}

int main(void) {
  /*
   * 配置 GPF4, GPF5, GPF6 为输出模式
   */
  GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
  GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));

  // 验证数据段是否正确被重定位
  if (g_j != 10) {
    // 如果 g_j 不是 10，说明 .data 段未正确初始化或重定位失败
    // 让 LED 全亮表示错误 (低电平亮)
    GPFDAT &= ~((1 << 4) | (1 << 5) | (1 << 6));
    while (1); 
  }

  while (1) {
    g_i++; // 在调试器中可观察此变量增加

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
