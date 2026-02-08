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

/* GPIO - PORT F */
#define GPFCON						__REG(0x56000050)
#define GPFDAT						__REG(0x56000054)

/* GPIO - PORT G */
#define GPGCON						__REG(0x56000060)
#define GPGDAT						__REG(0x56000064)

#endif /* __S3C2440_SOC_H__ */
