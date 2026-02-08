#ifndef __S3C2440_SOC_H__
#define __S3C2440_SOC_H__

#define __REG(x)					(*(volatile unsigned int *)(x))

/* WATCHDOG */
#define WTCON						__REG(0x53000000)

/* CLOCK & POWER MANAGEMENT */
#define LOCKTIME					__REG(0x4C000000)
#define MPLLCON						__REG(0x4C000004)
#define CLKDIVN						__REG(0x4C000014)

/* GPIO - PORT F */
#define GPFCON						__REG(0x56000050)
#define GPFDAT						__REG(0x56000054)

/* GPIO - PORT G */
#define GPGCON						__REG(0x56000060)
#define GPGDAT						__REG(0x56000064)

#endif /* __S3C2440_SOC_H__ */
