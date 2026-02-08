#include "s3c2440_soc.h"

void delay(volatile int d) {
  while (d--)
    ;
}

int main(void) {
  /*
   * 配置 GPF4, GPF5, GPF6 为输出模式 (LED)
   * GPFCON [9:8]   = 01 (GPF4 output)
   * GPFCON [11:10] = 01 (GPF5 output)
   * GPFCON [13:12] = 01 (GPF6 output)
   * 
   * 配置 GPF0, GPF2 为输入模式 (S2, S3)
   * GPFCON [1:0]   = 00 (GPF0 input)
   * GPFCON [5:4]   = 00 (GPF2 input)
   * 
   * 配置 GPG3, GPG11 为输入模式 (S4, S5)
   * GPGCON [7:6]   = 00 (GPG3 input)
   * GPGCON [23:22] = 00 (GPG11 input)
   */
  GPFCON &= ~((3 << 0) | (3 << 4) | (3 << 8) | (3 << 10) | (3 << 12));
  GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));

  GPGCON &= ~((3 << 6) | (3 << 22));

  /* 
   * 开启内部上拉电阻 (0: Enable, 1: Disable)
   * GPF0, GPF2, GPG3, GPG11
   */
  GPFUP &= ~((1 << 0) | (1 << 2));
  GPGUP &= ~((1 << 3) | (1 << 11));

  while (1) {
    unsigned int val_f = GPFDAT;
    unsigned int val_g = GPGDAT;

    // S2 (GPF0) 控制 LED1 (GPF4)
    if (!(val_f & (1 << 0))) {
      // 检测到按下，延时消抖
      delay(10000); 
      if (!(GPFDAT & (1 << 0))) {
        GPFDAT &= ~(1 << 4); // 点亮
      }
    } else {
      GPFDAT |= (1 << 4);    // 熄灭
    }

    // S3 (GPF2) 控制 LED2 (GPF5)
    if (!(val_f & (1 << 2))) {
      // 检测到按下，延时消抖
      delay(10000);
      if (!(GPFDAT & (1 << 2))) {
        GPFDAT &= ~(1 << 5); // 点亮
      }
    } else {
      GPFDAT |= (1 << 5);    // 熄灭
    }

    // S4 (GPG3) 或 S5 (GPG11) 控制 LED4 (GPF6)
    if (!(val_g & (1 << 3)) || !(val_g & (1 << 11))) {
      // 检测到按下，延_delay_消抖
      delay(10000);
      if (!(GPGDAT & (1 << 3)) || !(GPGDAT & (1 << 11))) {
        GPFDAT &= ~(1 << 6); // 点亮
      }
    } else {
      GPFDAT |= (1 << 6);    // 熄灭
    }
  }

  return 0;
}