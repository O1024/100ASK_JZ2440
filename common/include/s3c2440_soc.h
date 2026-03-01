/**
 * @file s3c2440_soc.h
 * @brief S3C2440 SoC Register Map
 * 
 * Copyright (c) 2026 JZ2440 Unified SDK Contributors
 * Distributed under the MIT License.
 */

#ifndef __S3C2440_SOC_H__
#define __S3C2440_SOC_H__

/* GPIO 寄存器 */
#define GPFCON      (*(volatile unsigned long *)0x56000050)
#define GPFDAT      (*(volatile unsigned long *)0x56000054)
#define GPFUP       (*(volatile unsigned long *)0x56000058)

#define GPGCON      (*(volatile unsigned long *)0x56000060)
#define GPGDAT      (*(volatile unsigned long *)0x56000064)
#define GPGUP       (*(volatile unsigned long *)0x56000068)

/* UART 寄存器 (以 UART0 为例) */
#define ULCON0      (*(volatile unsigned long *)0x50000000)
#define UCON0       (*(volatile unsigned long *)0x50000004)
#define UFCON0      (*(volatile unsigned long *)0x50000008)
#define UMCON0      (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0    (*(volatile unsigned long *)0x50000010)
#define UTXH0       (*(volatile unsigned char *)0x50000020)
#define URXH0       (*(volatile unsigned char *)0x50000024)
#define UBRDIV0     (*(volatile unsigned long *)0x50000028)

/* 看门狗寄存器 */
#define WTCON       (*(volatile unsigned long *)0x53000000)

#endif // __S3C2440_SOC_H__
