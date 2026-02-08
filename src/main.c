#include "s3c2440_soc.h"

/* 声明汇编定义的开启中断函数 */
void enable_irq(void);

/* 软件延时函数 */
void delay(volatile int d) {
  while (d--)
    ;
}

void key_eint_init(void) {
  /* 1. 引脚配置 (中断功能) */
  GPFCON = (GPFCON & ~((3 << 0) | (3 << 4))) | ((2 << 0) | (2 << 4));
  GPGCON = (GPGCON & ~((3 << 6) | (3 << 22))) | ((2 << 6) | (2 << 22));

  /* 2. 开启内部上拉电阻 */
  GPFUP &= ~((1 << 0) | (1 << 2));
  GPGUP &= ~((1 << 3) | (1 << 11));

  /* 3. 触发方式 (下降沿) */
  EXTINT0 = (EXTINT0 & ~((7 << 0) | (7 << 8))) | ((2 << 0) | (2 << 8));
  EXTINT1 = (EXTINT1 & ~(7 << 12)) | (2 << 12);
  EXTINT2 = (EXTINT2 & ~(7 << 12)) | (2 << 12);

  /* 4. 开启屏蔽位 */
  EINTMASK &= ~((1 << 11) | (1 << 19));
  INTMSK &= ~((1 << 0) | (1 << 2) | (1 << 5));
}

void handle_irq(void) {
  unsigned int offset = INTOFFSET;
  unsigned int eintpnd_val = 0;

  /* 1. 消除机械抖动：使用软件延时 */
  delay(20000);

  /* 2. 根据中断源处理逻辑 */
  if (offset == 0) { /* EINT0 (S2) */
    if (!(GPFDAT & (1 << 0))) {
      GPFDAT ^= (1 << 4); /* 翻转 LED1 */
    }
  } 
  else if (offset == 2) { /* EINT2 (S3) */
    if (!(GPFDAT & (1 << 2))) {
      GPFDAT ^= (1 << 5); /* 翻转 LED2 */
    }
  } 
  else if (offset == 5) { /* EINT8_23 (S4/S5) */
    eintpnd_val = EINTPND;
    
    if (eintpnd_val & (1 << 11)) { /* S4 */
      if (!(GPGDAT & (1 << 3))) {
        GPFDAT ^= (1 << 6);
      }
    }
    
    if (eintpnd_val & (1 << 19)) { /* S5 */
      if (!(GPGDAT & (1 << 11))) {
        GPFDAT ^= (1 << 4) | (1 << 5) | (1 << 6);
      }
    }

    /* 清除二级中断源 */
    EINTPND = eintpnd_val;
  }

  /* 3. 清除一级中断源 */
  SRCPND = (1 << offset);
  INTPND = (1 << offset);
}

int main(void) {
  /* LED 初始化 */
  GPFCON = (GPFCON & ~((3 << 8) | (3 << 10) | (3 << 12))) | ((1 << 8) | (1 << 10) | (1 << 12));
  GPFDAT |= (1 << 4) | (1 << 5) | (1 << 6);

  key_eint_init();
  enable_irq();

  while (1) {
    /* 主循环 */
  }
  return 0;
}