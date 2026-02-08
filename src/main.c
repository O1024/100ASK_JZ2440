#include "s3c2440_soc.h"

#define SRC_ADDR (0x30100000)
#define DST_ADDR (0x30300000)
#define TTB_ADDR (0x33DF0000)

static const char* labels[] = {
    "CPU (None) ", "CPU (I-On) ", "CPU (I+D)  ",
    "DMA (None) ", "DMA (I-On) ", "DMA (I+D)  "
};

/* --- Cache & MMU Helpers --- */

void full_cache_flush(void) {
    unsigned int set, way, index;
    /* 彻底清理 D-Cache */
    for (way = 0; way < 64; way++) {
        for (set = 0; set < 8; set++) {
            index = (way << 26) | (set << 5);
            asm volatile ("mcr p15, 0, %0, c7, c14, 2" : : "r" (index)); 
        }
    }
    /* 清理 Write Buffer, 失效 I-Cache, TLB */
    asm volatile (
        "mov r0, #0\n"
        "mcr p15, 0, r0, c7, c10, 4\n" // Drain Write Buffer
        "mcr p15, 0, r0, c7, c5, 0\n"  // Invalidate I-Cache
        "mcr p15, 0, r0, c8, c7, 0\n"  // Invalidate TLB
        ::: "r0"
    );
}

void disable_caches(void) {
    unsigned int reg;
    full_cache_flush();
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n"
        "bic %0, %0, #0x1000\n"
        "bic %0, %0, #0x0004\n"
        "bic %0, %0, #0x0001\n"
        "mcr p15, 0, %0, c1, c0, 0\n"
        : "=r" (reg)
    );
    full_cache_flush();
}

void enable_icache(void) {
    unsigned int reg;
    asm volatile ("mrc p15, 0, %0, c1, c0, 0\n" "orr %0, %0, #0x1000\n" "mcr p15, 0, %0, c1, c0, 0\n" : "=r" (reg));
}

void enable_all_caches(void) {
    unsigned int reg;
    unsigned int *ttb = (unsigned int *)TTB_ADDR;
    for (int i = 0; i < 4096; i++) ttb[i] = (i << 20) | (3 << 10) | 0x12; 
    for (int i = 0x300; i < 0x340; i++) ttb[i] = (i << 20) | (3 << 10) | 0x1E; 

    asm volatile (
        "mov r0, #0\n"
        "mcr p15, 0, r0, c7, c7, 0\n"
        "mcr p15, 0, r0, c8, c7, 0\n"
        "mcr p15, 0, %1, c2, c0, 0\n"
        "mvn r0, #0\n"
        "mcr p15, 0, r0, c3, c0, 0\n"
        "mrc p15, 0, %0, c1, c0, 0\n"
        "ldr r0, =0x1005\n"
        "orr %0, %0, r0\n"
        "mcr p15, 0, %0, c1, c0, 0\n"
        : "=r" (reg) : "r" (TTB_ADDR) : "r0"
    );
}

/* --- Drivers --- */

void timer_init(void) {
    TCFG0 = (TCFG0 & ~(0xFF << 8)) | (249 << 8);
    TCFG1 = (TCFG1 & ~(0xF << 16)) | (3 << 16); 
    TCNTB4 = 0xFFFF;
    TCON = (TCON & ~(0xF << 20)) | (1 << 21);
    TCON &= ~(1 << 21);
    TCON |= (1 << 22);
}

void timer_start(void) {
    TCON &= ~(1 << 20); TCNTB4 = 0xFFFF;
    TCON |= (1 << 21); TCON &= ~(1 << 21);
    TCON |= (1 << 22) | (1 << 20);
}

unsigned int timer_stop(void) {
    unsigned int val = TCNTO4; TCON &= ~(1 << 20); 
    return (0xFFFF - val); 
}

void uart_init(void) {
    GPHCON = (GPHCON & ~((3<<4) | (3<<6))) | ((2<<4) | (2<<6));
    GPHUP &= ~((1<<2) | (1<<3)); ULCON0 = 0x3; UCON0 = 0x5; UBRDIV0 = 26;
}

void puts(const char *s) { while (*s) { while (!(UTRSTAT0 & (1<<2))); UTXH0 = *s++; } }

void putdec(unsigned int val) {
    char buf[10]; int i = 0;
    if (val == 0) { puts("0"); return; }
    while (val > 0) { buf[i++] = (val % 10) + '0'; val /= 10; }
    while (i > 0) { while (!(UTRSTAT0 & (1<<2))); UTXH0 = buf[--i]; }
}

void print_perf(const char *label, unsigned int size_kb, unsigned int ticks, int verify) {
    unsigned int time_us = ticks * 80;
    puts(label); puts(" | ");
    
    if (time_us < 1000) {
        putdec(time_us); puts(" us   | ");
    } else {
        putdec(time_us / 1000); puts("."); putdec((time_us % 1000) / 100); puts(" ms | ");
    }
    
    if (verify == 0) puts("PASS   | "); else puts("FAIL   | ");
    
    if (time_us > 0) {
        unsigned int bits = size_kb * 8192;
        unsigned int mbps_int = bits / time_us;
        unsigned int mbps_frac = (bits * 100 / time_us) % 100;
        putdec(mbps_int); puts(".");
        if (mbps_frac < 10) puts("0"); 
        putdec(mbps_frac); puts(" Mbps\r\n");
    } else puts("N/A\r\n");
}

void init_buffers(unsigned int word_count) {
    volatile unsigned int *p_src = (volatile unsigned int *)SRC_ADDR;
    volatile unsigned int *p_dst = (volatile unsigned int *)DST_ADDR;
    for (unsigned int i = 0; i < word_count; i++) {
        p_src[i] = 0x55AA55AA + i; p_dst[i] = 0xAAAAAAAA;
    }
}

int verify_copy(unsigned int word_count) {
    volatile unsigned int *p_dst = (volatile unsigned int *)DST_ADDR;
    for (unsigned int i = 0; i < word_count; i++) {
        if (p_dst[i] != (0x55AA55AA + i)) return -1;
    }
    return 0;
}

void cpu_memcpy(unsigned int word_count) {
    volatile unsigned int *p_src = (volatile unsigned int *)SRC_ADDR;
    volatile unsigned int *p_dst = (volatile unsigned int *)DST_ADDR;
    for (unsigned int i = 0; i < word_count; i++) p_dst[i] = p_src[i];
}

void dma_memcpy(unsigned int word_count) {
    DMASKTRIG3 = 0;
    DISRC3 = SRC_ADDR; DISRCC3 = 0;
    DIDST3 = DST_ADDR; DIDSTC3 = 0;
    DCON3 = (0<<31)|(1<<30)|(1<<28)|(1<<27)|(0<<23)|(1<<22)|(2<<20)|word_count;
    DMASKTRIG3 = (1<<1) | (1<<0);
    while ((DSTAT3 & (1<<20)));
}

void run_test_suite(unsigned int size_kb) {
    unsigned int t;
    unsigned int words = size_kb * 1024 / 4;

    puts("\r\n>>> Testing Size: "); putdec(size_kb); puts(" KB\r\n");
    puts("Method      | Time      | Verify | Speed\r\n");
    puts("--------------------------------------------\r\n");

    /* 1. CPU (None) */
    disable_caches(); init_buffers(words);
    timer_start(); cpu_memcpy(words); t = timer_stop();
    print_perf(labels[0], size_kb, t, verify_copy(words));

    /* 2. CPU (I-On) */
    disable_caches(); enable_icache(); init_buffers(words);
    timer_start(); cpu_memcpy(words); t = timer_stop();
    print_perf(labels[1], size_kb, t, verify_copy(words));

    /* 3. CPU (I+D) */
    enable_all_caches(); init_buffers(words);
    full_cache_flush();
    timer_start(); cpu_memcpy(words); t = timer_stop();
    full_cache_flush();
    print_perf(labels[2], size_kb, t, verify_copy(words));

    /* 4. DMA (None) */
    disable_caches(); init_buffers(words);
    timer_start(); dma_memcpy(words); t = timer_stop();
    print_perf(labels[3], size_kb, t, verify_copy(words));

    /* 5. DMA (I-On) */
    disable_caches(); enable_icache(); init_buffers(words);
    timer_start(); dma_memcpy(words); t = timer_stop();
    print_perf(labels[4], size_kb, t, verify_copy(words));

    /* 6. DMA (I+D) */
    enable_all_caches(); init_buffers(words);
    full_cache_flush();
    timer_start(); dma_memcpy(words); t = timer_stop();
    full_cache_flush();
    print_perf(labels[5], size_kb, t, verify_copy(words));

    puts("--------------------------------------------\r\n");
}

int main(void) {
    uart_init(); timer_init();
    *(volatile unsigned int *)0x4C00000C |= (1 << 14);

    while (1) {
        puts("\r\n=== S3C2440 Memcpy Performance Full Bench ===\r\n");

        run_test_suite(4);
        run_test_suite(16);
        run_test_suite(64);
        run_test_suite(256);
        run_test_suite(1024);

        volatile int d = 10000000; while(d--);
    }
    return 0;
}