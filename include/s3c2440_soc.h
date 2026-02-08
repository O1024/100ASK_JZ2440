#ifndef __S3C2440_SOC_H__
#define __S3C2440_SOC_H__

#define __REG(x)					(*(volatile unsigned int *)(x))

/* WATCHDOG */
#define WTCON						__REG(0x53000000)

/* GPIO - PORT F */
#define GPFCON						__REG(0x56000050)
#define GPFDAT						__REG(0x56000054)
#define GPFUP						__REG(0x56000058)

/* GPIO - PORT G */
#define GPGCON						__REG(0x56000060)
#define GPGDAT						__REG(0x56000064)
#define GPGUP						__REG(0x56000068)

/* GPIO - PORT H */
#define GPHCON						__REG(0x56000070)
#define GPHDAT						__REG(0x56000074)
#define GPHUP						__REG(0x56000078)

/* UART 0 */
#define ULCON0						__REG(0x50000000)
#define UCON0						__REG(0x50000004)
#define UFCON0						__REG(0x50000008)
#define UMCON0						__REG(0x5000000C)
#define UTRSTAT0					__REG(0x50000010)
#define UERSTAT0					__REG(0x50000014)
#define UFSTAT0						__REG(0x50000018)
#define UMSTAT0						__REG(0x5000001C)
#define UTXH0						__REG(0x50000020)
#define URXH0						(*(volatile unsigned char *)(0x50000024))
#define UBRDIV0						__REG(0x50000028)

#endif /* __S3C2440_SOC_H__ */