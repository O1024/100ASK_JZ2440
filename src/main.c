#include "s3c2440_soc.h"

#define BUF_SIZE (1024 * 1024 / 4) // 1MB
#define SRC_ADDR (0x30100000)
#define DST_ADDR (0x30200000)

#define CLKCON (*(volatile unsigned int *)0x4C00000C)

/* Timer 4: 用于性能测量 (1MHz 频率) */
void timer_init(void) {
    /* PCLK = 50MHz
     * Prescaler 1 (for Timer 2,3,4) = 49
     * Timer 4 clock = 50MHz / (49 + 1) = 1MHz
     */
    TCFG0 &= ~(0xFF << 8);
    TCFG0 |= (49 << 8);
    
    /* Divider for Timer 4 = 1/1 */
    TCFG1 &= ~(0xF << 16);
    
    /* 设置初始值 (最大值，向下计数) */
    TCNTB4 = 0xFFFF;
    
    /* 手动更新 TCNTB4 到计数器 */
    TCON |= (1 << 22);
    /* 清除手动更新位，设置为自动重载并停止 */
    TCON &= ~(1 << 22);
    TCON |= (1 << 21); // Auto-reload
}

void timer_start(void) {
    TCNTB4 = 0xFFFF;
    TCON |= (1 << 22); // Manual update
    TCON &= ~(1 << 22);
    TCON |= (1 << 20); // Start timer 4
}

unsigned int timer_stop(void) {
    TCON &= ~(1 << 20); // Stop timer 4
    return (0xFFFF - TCNTO4); // 返回消耗的 ticks (微秒)
}

void uart_init(void) {
    GPHCON &= ~((3<<4) | (3<<6));
    GPHCON |= ((2<<4) | (2<<6));
    GPHUP &= ~((1<<2) | (1<<3));
    ULCON0 = 0x3; UCON0 = 0x5; UFCON0 = 0x0; UMCON0 = 0x0;
    UBRDIV0 = 26;
}

void putchar(char c) {
    while (!(UTRSTAT0 & (1<<2)));
    UTXH0 = c;
}

void puts(const char *s) {
    while (*s) putchar(*s++);
}

void putdec(unsigned int val) {
    char buf[10];
    int i = 0;
    if (val == 0) { putchar('0'); return; }
    while (val > 0) {
        buf[i++] = (val % 10) + '0';
        val /= 10;
    }
    while (i > 0) putchar(buf[--i]);
}

void delay(volatile int d) { while (d--); }

void init_buffers(void) {
    volatile unsigned int *p_src = (volatile unsigned int *)SRC_ADDR;
    volatile unsigned int *p_dst = (volatile unsigned int *)DST_ADDR;
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        p_src[i] = 0x55AA55AA + i;
        p_dst[i] = 0xAAAAAAAA;
    }
}

int verify_copy(void) {
    volatile unsigned int *p_dst = (volatile unsigned int *)DST_ADDR;
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        if (p_dst[i] != (0x55AA55AA + i)) return -1;
    }
    return 0;
}

void cpu_memcpy(void) {
    volatile unsigned int *p_src = (volatile unsigned int *)SRC_ADDR;
    volatile unsigned int *p_dst = (volatile unsigned int *)DST_ADDR;
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        p_dst[i] = p_src[i];
    }
}

void dma_memcpy(void) {
    DMASKTRIG3 = 0;
    DISRC3 = SRC_ADDR;
    DISRCC3 = 0; 
    DIDST3 = DST_ADDR;
    DIDSTC3 = 0; 
    DCON3 = (1<<31)|(1<<30)|(1<<28)|(1<<27)|(0<<23)|(1<<22)|(2<<20)|BUF_SIZE;
    DMASKTRIG3 = (1<<1) | (1<<0);
    while ((DSTAT3 & 0xFFFFF) != 0);
}

int main(void) {
    unsigned int time_cpu, time_dma;
    uart_init();
    timer_init();
    CLKCON |= (1 << 14);

    GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
    GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));
    GPFDAT |= (7 << 4);

    puts("\r\n=== S3C2440 Memcpy Performance Test (1MB) ===\r\n");

    while (1) {
        /* 1. CPU Test */
        init_buffers();
        puts("CPU Memcpy running... ");
        timer_start();
        cpu_memcpy();
        time_cpu = timer_stop();
        puts("Done.\r\n");

        /* 2. DMA Test */
        init_buffers();
        puts("DMA Memcpy running... ");
        timer_start();
        dma_memcpy();
        time_dma = timer_stop();
        puts("Done.\r\n");

        /* Results */
        puts("--------------------------------------\r\n");
        puts("CPU Time: "); putdec(time_cpu); puts(" us\r\n");
        puts("DMA Time: "); putdec(time_dma); puts(" us\r\n");
        
        if (verify_copy() == 0) {
            puts("Verification: SUCCESS\r\n");
            GPFDAT &= ~(1 << 6); delay(500000); GPFDAT |= (1 << 6);
        } else {
            puts("Verification: FAILED\r\n");
            GPFDAT &= ~(7 << 4); delay(100000); GPFDAT |= (7 << 4); delay(100000);
        }
        puts("--------------------------------------\r\n\r\n");
        
        delay(3000000);
    }
    return 0;
}
