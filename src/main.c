#include "s3c2440_soc.h"

/* 实验参数：1MB 数据量 (256K 个 Word) */
#define BUF_SIZE (1024 * 1024 / 4)
#define SRC_ADDR (0x30100000)
#define DST_ADDR (0x30200000)

#define CLKCON (*(volatile unsigned int *)0x4C00000C)

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

void puthex(unsigned int val) {
    int i;
    char arr[16] = "0123456789ABCDEF";
    puts("0x");
    for (i = 0; i < 8; i++) putchar(arr[(val >> ((7-i)*4)) & 0xF]);
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

    /* 
     * S3C2440 DCON3 关键配置:
     * [31] Handshake=1, [30] Sync=1, [28] TSZ=1 (Burst4), [27] Whole=1
     * [23] SWHW_SEL=0 (Software Request) - 陷阱：必须为0
     * [22] RELOAD=1 (Off) - 陷阱：必须为1
     * [21:20] DSZ=10 (Word)
     */
    DCON3 = (1<<31)|(1<<30)|(1<<28)|(1<<27)|(0<<23)|(1<<22)|(2<<20)|BUF_SIZE;

    /* 触发 */
    DMASKTRIG3 = (1<<1) | (1<<0);

    /* 轮询直到 TC 归零 */
    while ((DSTAT3 & 0xFFFFF) != 0);
}

int main(void) {
    uart_init();
    CLKCON |= (1 << 14); /* 开启 DMA 时钟 */

    GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
    GPFCON |= ((1 << 8) | (1 << 10) | (1 << 12));
    GPFDAT |= (7 << 4);

    puts("\r\n=== DMA Performance Lab ===\r\n");

    while (1) {
        init_buffers();
        
        /* 1. CPU Memcpy */
        puts("CPU Copy Start... ");
        GPFDAT &= ~(1 << 4); /* LED1 ON */
        cpu_memcpy();
        GPFDAT |= (1 << 4);  /* LED1 OFF */
        puts("Done.\r\n");

        if (verify_copy() != 0) { puts("CPU Copy Verify FAILED!\r\n"); goto error; }

        delay(1000000);
        init_buffers();

        /* 2. DMA Memcpy */
        puts("DMA Copy Start... ");
        GPFDAT &= ~(1 << 5); /* LED2 ON */
        dma_memcpy();
        GPFDAT |= (1 << 5);  /* LED2 OFF */
        puts("Done.\r\n");

        if (verify_copy() == 0) {
            puts("SUCCESS: DMA is working!\r\n");
            GPFDAT &= ~(1 << 6); delay(500000); GPFDAT |= (1 << 6);
        } else {
            puts("FAIL: DMA verify failed!\r\n");
            goto error;
        }
        
        puts("Wait for next round...\r\n\r\n");
        delay(3000000);
    }

error:
    while (1) {
        GPFDAT &= ~(7 << 4); delay(100000); GPFDAT |= (7 << 4); delay(100000);
    }
    return 0;
}