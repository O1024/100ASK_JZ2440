#include "s3c2440_soc.h"

#define BUF_SIZE (1024 * 1024 / 4) // 1MB / 4 bytes = 256K words
volatile unsigned int *src_buf = (volatile unsigned int *)0x30100000;
volatile unsigned int *dst_buf = (volatile unsigned int *)0x30200000;

void delay(volatile int d) {
  while (d--)
    ;
}

void init_buffers(void) {
  int i;
  for (i = 0; i < BUF_SIZE; i++) {
    src_buf[i] = 0xDEADBEEF + i;
    dst_buf[i] = 0;
  }
}

int verify_copy(void) {
  int i;
  for (i = 0; i < BUF_SIZE; i++) {
    if (dst_buf[i] != (0xDEADBEEF + i)) {
      return -1; // Fail
    }
  }
  return 0; // Success
}

void cpu_memcpy(void) {
  int i;
  for (i = 0; i < BUF_SIZE; i++) {
    dst_buf[i] = src_buf[i];
  }
}

void dma_memcpy(void) {
  /* 1. 设置源地址 */
  DISRC0 = (unsigned int)src_buf;
  /* 2. 源控制: AHB总线, 地址自增 */
  DISRCC0 = (0 << 1) | (0 << 0);
  
  /* 3. 设置目的地址 */
  DIDST0 = (unsigned int)dst_buf;
  /* 4. 目的控制: AHB总线, 地址自增 */
  DIDSTC0 = (0 << 1) | (0 << 0);

  /* 
   * 5. 设置控制寄存器 (DCON0)
   * [31]: 1 (Handshake mode - though for SW it's less critical)
   * [30]: 1 (Sync PCLK)
   * [29]: 1 (Interrupt enable)
   * [28]: 1 (Burst size: 4)
   * [27]: 1 (Whole service mode)
   * [26:24]: 000 (Software request)
   * [23]: 1 (Reload disable - finish after one transfer)
   * [22:20]: 010 (Data size: Word)
   * [19:0]: BUF_SIZE (Transfer count)
   */
  DCON0 = (1 << 31) | (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) | \
          (0 << 24) | (1 << 23) | (2 << 20) | (BUF_SIZE);

  /* 6. 启动 DMA: 设置 DMASKTRIG [1]=1, [0]=1 (SW trigger) */
  DMASKTRIG0 = (1 << 1) | (1 << 0);

  /* 7. 等待 DMA 完成: 轮询 DSTAT0 [20] (0 = Busy, 1 = Done?) 
   * 其实 S3C2440 DMA 完成后 DMASKTRIG [1] 会变回 0 
   */
  while (DSTAT0 & (1 << 20)) // Wait while busy (if status is 0, it is busy in some docs, check manual)
    ;
  // 或者轮询计数器是否归零
  while ((DSTAT0 & 0xFFFFF) != 0)
    ;
}

int main(void) {
  /* 配置 LED 引脚为输出 */
  GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
  GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));
  GPFDAT |= (7 << 4); // 全灭

  init_buffers();

  /* --- CPU 拷贝对比 --- */
  GPFDAT &= ~(1 << 4); // LED1 亮
  cpu_memcpy();
  GPFDAT |= (1 << 4);  // LED1 灭

  if (verify_copy() != 0) goto error;
  
  delay(1000000); // 间隔
  init_buffers(); // 重置

  /* --- DMA 拷贝对比 --- */
  GPFDAT &= ~(1 << 5); // LED2 亮
  dma_memcpy();
  GPFDAT |= (1 << 5);  // LED2 灭

  if (verify_copy() != 0) goto error;

  /* 成功：流水灯 */
  while (1) {
    GPFDAT = (GPFDAT | (7 << 4)) & ~(1 << 6);
    delay(500000);
    GPFDAT |= (7 << 4);
    delay(500000);
  }

error:
  /* 失败：爆闪 */
  while (1) {
    GPFDAT &= ~(7 << 4);
    delay(100000);
    GPFDAT |= (7 << 4);
    delay(100000);
  }

  return 0;
}