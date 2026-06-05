/**
 * @file s3c2440_soc.h
 * @brief Corrected S3C2440 Register Map.
 */

#ifndef __S3C2440_SOC_H__
#define __S3C2440_SOC_H__

#include <stdint.h>

#define MEM_CTL_BASE    0x48000000
#define INTC_BASE       0x4A000000
#define CLK_PWR_BASE    0x4C000000
#define LCD_CTL_BASE    0x4D000000
#define NAND_CTL_BASE   0x4E000000
#define UART_BASE       0x50000000
#define TIMER_BASE      0x51000000
#define WDT_BASE        0x53000000
#define GPIO_BASE       0x56000000
#define DMA_BASE        0x4B000000

typedef struct {
    volatile uint32_t DISRC;      // Initial source address
    volatile uint32_t DISRCC;     // Initial source control
    volatile uint32_t DIDST;      // Initial destination address
    volatile uint32_t DIDSTC;     // Initial destination control
    volatile uint32_t DCON;       // DMA control
    volatile uint32_t DSTAT;      // DMA status
    volatile uint32_t DCSRC;      // Current source address
    volatile uint32_t DCDST;      // Current destination address
    volatile uint32_t DMASKTRIG;  // Mask and trigger
} dma_ch_t;

typedef struct {
    dma_ch_t CH[4];
} dma_t;

typedef struct {
    volatile uint32_t SRCPND;
    volatile uint32_t INTMOD;
    volatile uint32_t INTMSK;
    volatile uint32_t PRIORITY;
    volatile uint32_t INTPND;
    volatile uint32_t INTOFFSET;
    volatile uint32_t SUBSRCPND;
    volatile uint32_t INTSUBMSK;
} intc_t;

typedef struct {
    volatile uint32_t TCFG0;    /* 0x00 */
    volatile uint32_t TCFG1;    /* 0x04 */
    volatile uint32_t TCON;     /* 0x08 */
    volatile uint32_t TCNTB0;   /* 0x0C */
    volatile uint32_t TCMPB0;   /* 0x10 */
    volatile uint32_t TCNTO0;   /* 0x14 */
    volatile uint32_t TCNTB1;   /* 0x18 */
    volatile uint32_t TCMPB1;   /* 0x1C */
    volatile uint32_t TCNTO1;   /* 0x20 */
    volatile uint32_t TCNTB2;   /* 0x24 */
    volatile uint32_t TCMPB2;   /* 0x28 */
    volatile uint32_t TCNTO2;   /* 0x2C */
    volatile uint32_t TCNTB3;   /* 0x30 */
    volatile uint32_t TCMPB3;   /* 0x34 */
    volatile uint32_t TCNTO3;   /* 0x38 */
    volatile uint32_t TCNTB4;   /* 0x3C - Correct! */
    volatile uint32_t TCNTO4;   /* 0x40 */
    volatile uint32_t TINT_CSTAT; /* 0x44 */
} timer_ctl_t;

typedef struct {
    volatile uint32_t BWSCON;
    volatile uint32_t BANKCON[8];
    volatile uint32_t REFRESH;
    volatile uint32_t BANKSIZE;
    volatile uint32_t MRSRB6;
    volatile uint32_t MRSRB7;
} mem_ctl_t;

typedef struct {
    volatile uint32_t LOCKTIME;
    volatile uint32_t MPLLCON;
    volatile uint32_t UPLLCON;
    volatile uint32_t CLKCON;
    volatile uint32_t CLKSLOW;
    volatile uint32_t CLKDIVN;
    volatile uint32_t CAMDIVN;
} clk_pwr_t;

typedef struct {
    volatile uint32_t LCDCON1;
    volatile uint32_t LCDCON2;
    volatile uint32_t LCDCON3;
    volatile uint32_t LCDCON4;
    volatile uint32_t LCDCON5;
    volatile uint32_t LCDSADDR1;
    volatile uint32_t LCDSADDR2;
    volatile uint32_t LCDSADDR3;
    volatile uint32_t REDLUT;
    volatile uint32_t GREENLUT;
    volatile uint32_t BLUELUT;
    volatile uint32_t _pad0[8];
    volatile uint32_t DITHMODE;
    volatile uint32_t TPAL;
} lcd_ctl_t;

typedef struct {
    volatile uint32_t NFCONF;
    volatile uint32_t NFCONT;
    volatile uint8_t  NFCMMD;
    volatile uint8_t  _pad0[3];
    volatile uint8_t  NFADDR;
    volatile uint8_t  _pad1[3];
    volatile uint8_t  NFDATA;
    volatile uint8_t  _pad2[3];
    volatile uint32_t NFMECCD0;
    volatile uint32_t NFMECCD1;
    volatile uint32_t NFSECCD;
    volatile uint32_t NFSTAT;
    volatile uint32_t NFESTAT0;
    volatile uint32_t NFESTAT1;
    volatile uint32_t NFMECC0;
    volatile uint32_t NFMECC1;
    volatile uint32_t NFSECC;
    volatile uint32_t NFMLCBITPT;
} nand_t;

typedef struct {
    volatile uint32_t ULCON;
    volatile uint32_t UCON;
    volatile uint32_t UFCON;
    volatile uint32_t UMCON;
    volatile uint32_t UTRSTAT;
    volatile uint32_t UERSTAT;
    volatile uint32_t UFSTAT;
    volatile uint32_t UMSTAT;
    volatile uint8_t  UTXH;
    volatile uint8_t  _pad0[3];
    volatile uint8_t  URXH;
    volatile uint8_t  _pad1[3];
    volatile uint32_t UBRDIV;
} uart_t;

typedef struct {
    volatile uint32_t CON;
    volatile uint32_t DAT;
    volatile uint32_t UP;
    volatile uint32_t RSVD;
} gpio_port_t;

#define MEM_CTL         ((mem_ctl_t *)MEM_CTL_BASE)
#define CLK_PWR         ((clk_pwr_t *)CLK_PWR_BASE)
#define LCD             ((lcd_ctl_t *)LCD_CTL_BASE)
#define NAND            ((nand_t *)NAND_CTL_BASE)
#define UART0           ((uart_t *)UART_BASE)
#define UART1           ((uart_t *)(UART_BASE + 0x4000))
#define UART2           ((uart_t *)(UART_BASE + 0x8000))
#define INTC            ((intc_t *)INTC_BASE)
#define TIMER           ((timer_ctl_t *)TIMER_BASE)
#define DMA             ((dma_t *)DMA_BASE)
#define GPIO_PORT(n)    ((gpio_port_t *)(GPIO_BASE + (n) * 0x10))
#define WDT_CON         (*(volatile uint32_t *)WDT_BASE)

#define PORT_A  0
#define PORT_B  1
#define PORT_C  2
#define PORT_D  3
#define PORT_E  4
#define PORT_F  5
#define PORT_G  6
#define PORT_H  7

#endif
