#include "s3c2440_soc.h"

#define SRC_ADDR (0x30100000)
#define DST_ADDR (0x30300000)
#define TTB_ADDR (0x33DF0000)

static const char* labels[] = {
    "CPU (None)     ", "CPU (I-On)     ", "CPU (I+D)      ",
    "CPU (C-Unroll) ", "CPU (ASM-32B)  ", "CPU (ASM-128B) ",
    "DMA (None)     ", "DMA (I-On)     ", "DMA (I+D)      "
};

/* --- Helpers --- */
void full_cache_flush(void) {
    unsigned int set, way, index;
    for (way = 0; way < 64; way++) {
        for (set = 0; set < 8; set++) {
            index = (way << 26) | (set << 5);
            asm volatile ("mcr p15, 0, %0, c7, c14, 2" : : "r" (index)); 
        }
    }
    asm volatile (
        "mov r0, #0\n"
        "mcr p15, 0, r0, c7, c10, 4\n"
        "mcr p15, 0, r0, c7, c5, 0\n"
        "mcr p15, 0, r0, c8, c7, 0\n"
        ::: "r0"
    );
}

void disable_caches(void) {
    unsigned int reg;
    full_cache_flush();
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n"
        "bic %0, %0, #0x1000\n"
        "bic %0, %0, #0x0005\n"
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
        "mcr p15, 0, %1, c2, c0, 0\n"
        "mvn r0, #0\n mcr p15, 0, r0, c3, c0, 0\n"
        "mrc p15, 0, %0, c1, c0, 0\n"
        "ldr r0, =0x1005\n"
        "orr %0, %0, r0\n"
        "mcr p15, 0, %0, c1, c0, 0\n"
        : "=r" (reg) : "r" (TTB_ADDR) : "r0"
    );
}

/* --- Optimized CPU Copy --- */
void cpu_memcpy_unroll(unsigned int n) {
    volatile unsigned int *s=(volatile unsigned int*)SRC_ADDR, *d=(volatile unsigned int*)DST_ADDR;
    for(unsigned int i=0; i<n; i+=8) { d[i]=s[i]; d[i+1]=s[i+1]; d[i+2]=s[i+2]; d[i+3]=s[i+3]; d[i+4]=s[i+4]; d[i+5]=s[i+5]; d[i+6]=s[i+6]; d[i+7]=s[i+7]; }
}
void cpu_memcpy_asm_32(unsigned int n) {
    unsigned int b = n/8; volatile unsigned int *s=(volatile unsigned int*)SRC_ADDR, *d=(volatile unsigned int*)DST_ADDR;
    asm volatile ("1: ldmia %0!, {r3-r10}\n stmia %1!, {r3-r10}\n subs %2, %2, #1\n bne 1b\n" : "+r"(s), "+r"(d), "+r"(b) :: "r3","r4","r5","r6","r7","r8","r9","r10","memory");
}
void cpu_memcpy_asm_128(unsigned int n) {
    unsigned int b = n/32; volatile unsigned int *s=(volatile unsigned int*)SRC_ADDR, *d=(volatile unsigned int*)DST_ADDR;
    asm volatile ("1: ldmia %0!, {r3-r10}\n stmia %1!, {r3-r10}\n ldmia %0!, {r3-r10}\n stmia %1!, {r3-r10}\n ldmia %0!, {r3-r10}\n stmia %1!, {r3-r10}\n ldmia %0!, {r3-r10}\n stmia %1!, {r3-r10}\n subs %2, %2, #1\n bne 1b\n" : "+r"(s), "+r"(d), "+r"(b) :: "r3","r4","r5","r6","r7","r8","r9","r10","memory");
}

/* --- Drivers --- */
void timer_init(void) { TCFG0 = (249<<8); TCFG1 = (3<<16); TCNTB4 = 0xFFFF; TCON = (TCON & ~(0xF<<20)) | (1<<21); TCON &= ~(1<<21); TCON |= (1<<22); }
void timer_start(void) { TCON &= ~(1<<20); TCNTB4 = 0xFFFF; TCON |= (1<<21); TCON &= ~(1<<21); TCON |= (1<<22) | (1<<20); }
unsigned int timer_stop(void) { unsigned int val = TCNTO4; TCON &= ~(1<<20); return (0xFFFF - val); }
void uart_init(void) { GPHCON=(GPHCON&~((3<<4)|(3<<6)))|((2<<4)|(2<<6)); GPHUP&=~((1<<2)|(1<<3)); ULCON0=3; UCON0=5; UBRDIV0=26; }
void putchar(char c) { while (!(UTRSTAT0 & (1<<2))); UTXH0 = c; }
void puts(const char *s) { while (*s) putchar(*s++); }
void putdec(unsigned int val) { char buf[10]; int i=0; if(val==0){puts("0");return;} while(val>0){buf[i++]=(val%10)+'0';val/=10;} while(i>0)putchar(buf[--i]); }

void init_buf(unsigned int n) { volatile unsigned int *s=(volatile unsigned int*)SRC_ADDR; for(unsigned int i=0; i<n; i++) s[i]=0x55AA55AA+i; }
int verify(unsigned int n) { volatile unsigned int *d=(volatile unsigned int*)DST_ADDR; for(unsigned int i=0; i<n; i++) if(d[i]!=0x55AA55AA+i) return -1; return 0; }

void print_perf(const char *label, unsigned int kb, unsigned int ticks, int pass) {
    unsigned int us = ticks * 80;
    puts(label); puts(" | ");
    if (us < 1000) { putdec(us); puts(" us   | "); } else { putdec(us/1000); puts("."); putdec((us%1000)/100); puts(" ms | "); }
    if (pass == 0) puts("PASS | "); else puts("FAIL | ");
    unsigned int bits = kb * 8192;
    putdec(bits/us); puts("."); unsigned int f=(bits*100/us)%100; if(f<10)putchar('0'); putdec(f); puts(" Mbps\r\n");
}

int main(void) {
    uart_init(); timer_init(); *(volatile unsigned int *)0x4C00000C |= (1 << 14);
    while (1) {
        puts("\r\n=== S3C2440 Memcpy Performance Ultimate ===\r\n");
        unsigned int skb = 1024, words = skb * 1024 / 4, t;

        disable_caches(); init_buf(words); timer_start();
        for(unsigned int i=0; i<words; i++) ((volatile unsigned int *)DST_ADDR)[i] = ((volatile unsigned int *)SRC_ADDR)[i];
        t = timer_stop(); print_perf(labels[0], skb, t, verify(words));

        disable_caches(); enable_icache(); init_buf(words); timer_start();
        for(unsigned int i=0; i<words; i++) ((volatile unsigned int *)DST_ADDR)[i] = ((volatile unsigned int *)SRC_ADDR)[i];
        t = timer_stop(); print_perf(labels[1], skb, t, verify(words));

        enable_all_caches(); init_buf(words); full_cache_flush(); timer_start();
        for(unsigned int i=0; i<words; i++) ((volatile unsigned int *)DST_ADDR)[i] = ((volatile unsigned int *)SRC_ADDR)[i];
        t = timer_stop(); full_cache_flush(); print_perf(labels[2], skb, t, verify(words));

        init_buf(words); full_cache_flush(); timer_start();
        cpu_memcpy_unroll(words); t = timer_stop(); full_cache_flush(); print_perf(labels[3], skb, t, verify(words));

        init_buf(words); full_cache_flush(); timer_start();
        cpu_memcpy_asm_32(words); t = timer_stop(); full_cache_flush(); print_perf(labels[4], skb, t, verify(words));

        init_buf(words); full_cache_flush(); timer_start();
        cpu_memcpy_asm_128(words); t = timer_stop(); full_cache_flush(); print_perf(labels[5], skb, t, verify(words));

        disable_caches(); init_buf(words); timer_start();
        DCON3=(0<<31)|(1<<30)|(1<<28)|(1<<27)|(0<<23)|(1<<22)|(2<<20)|words; DMASKTRIG3=3; while((DSTAT3&(1<<20)));
        t = timer_stop(); print_perf(labels[6], skb, t, verify(words));

        disable_caches(); enable_icache(); init_buf(words); timer_start();
        DCON3=(0<<31)|(1<<30)|(1<<28)|(1<<27)|(0<<23)|(1<<22)|(2<<20)|words; DMASKTRIG3=3; while((DSTAT3&(1<<20)));
        t = timer_stop(); print_perf(labels[7], skb, t, verify(words));

        enable_all_caches(); init_buf(words); full_cache_flush(); timer_start();
        DCON3=(0<<31)|(1<<30)|(1<<28)|(1<<27)|(0<<23)|(1<<22)|(2<<20)|words; DMASKTRIG3=3; while((DSTAT3&(1<<20)));
        t = timer_stop(); full_cache_flush(); print_perf(labels[8], skb, t, verify(words));

        puts("--------------------------------------------\r\n");
        volatile int d = 10000000; while(d--);
    }
    return 0;
}