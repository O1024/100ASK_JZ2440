#ifndef __S3C2440_SOC_H__
#define __S3C2440_SOC_H__

#define __REG(x)					(*(volatile unsigned int *)(x))

/* WATCHDOG */
#define WTCON						__REG(0x53000000)

/* MEMORY CONTROLLER */
#define BWSCON						__REG(0x48000000)
#define BANKCON0					__REG(0x48000004)
#define BANKCON1					__REG(0x48000008)
#define BANKCON2					__REG(0x4800000C)
#define BANKCON3					__REG(0x48000010)
#define BANKCON4					__REG(0x48000014)
#define BANKCON5					__REG(0x48000018)
#define BANKCON6					__REG(0x4800001C)
#define BANKCON7					__REG(0x48000020)
#define REFRESH						__REG(0x48000024)
#define BANKSIZE					__REG(0x48000028)
#define MRSRB6						__REG(0x4800002C)
#define MRSRB7						__REG(0x48000030)

/* DMA */
#define DISRC0						__REG(0x4B000000)
#define DISRCC0						__REG(0x4B000004)
#define DIDST0						__REG(0x4B000008)
#define DIDSTC0						__REG(0x4B00000C)
#define DCON0						__REG(0x4B000010)
#define DSTAT0						__REG(0x4B000014)
#define DCSRC0						__REG(0x4B000018)
#define DCDST0						__REG(0x4B00001C)
#define DMASKTRIG0					__REG(0x4B000020)

/* DMA 3 */
#define DISRC3						__REG(0x4B0000C0)
#define DISRCC3						__REG(0x4B0000C4)
#define DIDST3						__REG(0x4B0000C8)
#define DIDSTC3						__REG(0x4B0000CC)
#define DCON3						__REG(0x4B0000D0)
#define DSTAT3						__REG(0x4B0000D4)
#define DCSRC3						__REG(0x4B0000D8)
#define DCDST3						__REG(0x4B0000DC)
#define DMASKTRIG3					__REG(0x4B0000E0)

/* UART0 */
#define ULCON0                      __REG(0x50000000)
#define UCON0                       __REG(0x50000004)
#define UFCON0                      __REG(0x50000008)
#define UMCON0                      __REG(0x5000000C)
#define UTRSTAT0                    __REG(0x50000010)
#define UERSTAT0                    __REG(0x50000014)
#define UFSTAT0                     __REG(0x50000018)
#define UMSTAT0                     __REG(0x5000001C)
#define UTXH0                       __REG(0x50000020)
#define URXH0                       __REG(0x50000024)
#define UBRDIV0                     __REG(0x50000028)

/* PWM TIMERS */
#define TCFG0                       __REG(0x51000000)
#define TCFG1                       __REG(0x51000004)
#define TCON                        __REG(0x51000008)
#define TCNTB4                      __REG(0x5100003C)
#define TCNTO4                      __REG(0x51000040)

/* GPIO - PORT F */
#define GPFCON						__REG(0x56000050)
#define GPFDAT						__REG(0x56000054)

/* GPIO - PORT G */
#define GPGCON						__REG(0x56000060)
#define GPGDAT						__REG(0x56000064)

/* GPIO - PORT H */
#define GPHCON						__REG(0x56000070)
#define GPHDAT						__REG(0x56000074)
#define GPHUP						__REG(0x56000078)

#endif /* __S3C2440_SOC_H__ */
