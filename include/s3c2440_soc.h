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

/* INTERRUPTS */
#define SRCPND						__REG(0x4A000000)
#define INTMSK						__REG(0x4A000008)
#define INTPND						__REG(0x4A000010)
#define INTOFFSET					__REG(0x4A000014)

/* EXTERNAL INTERRUPTS */
#define EXTINT0						__REG(0x56000088)
#define EXTINT1						__REG(0x5600008C)
#define EXTINT2						__REG(0x56000090)
#define EINTMASK					__REG(0x560000A4)
#define EINTPND						__REG(0x560000A8)

/* PWM TIMERS */
#define TCFG0						__REG(0x51000000)
#define TCFG1						__REG(0x51000004)
#define TCON						__REG(0x51000008)
#define TCNTB0						__REG(0x5100000C)
#define TCMPB0						__REG(0x51000010)
#define TCNTO0						__REG(0x51000014)
#define TCNTB4						__REG(0x5100003C)
#define TCNTO4						__REG(0x51000040)

#endif /* __S3C2440_SOC_H__ */
