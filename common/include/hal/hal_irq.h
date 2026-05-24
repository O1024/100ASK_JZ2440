/**
 * @file hal_irq.h
 * @brief Hardware Abstraction Layer for S3C2440 Interrupt Controller
 */

#ifndef __HAL_IRQ_H__
#define __HAL_IRQ_H__

#include <stdint.h>

/* S3C2440 Interrupt Sources (Offset indices) */
#define IRQ_EINT0      0
#define IRQ_EINT1      1
#define IRQ_EINT2      2
#define IRQ_EINT3      3
#define IRQ_EINT4_7    4
#define IRQ_EINT8_23   5
#define IRQ_CAM        6
#define IRQ_BAT_FLT    7
#define IRQ_TICK       8
#define IRQ_WDT_AC97   9
#define IRQ_TIMER0     10
#define IRQ_TIMER1     11
#define IRQ_TIMER2     12
#define IRQ_TIMER3     13
#define IRQ_TIMER4     14
#define IRQ_UART2      15
#define IRQ_LCD        16
#define IRQ_DMA0       17
#define IRQ_DMA1       18
#define IRQ_DMA2       19
#define IRQ_DMA3       20
#define IRQ_SDI        21
#define IRQ_SPI0       22
#define IRQ_UART1      23
#define IRQ_NFCON      24
#define IRQ_USBD       25
#define IRQ_USBH       26
#define IRQ_IIC        27
#define IRQ_UART0      28
#define IRQ_SPI1       29
#define IRQ_RTC        30
#define IRQ_ADC        31

/**
 * @brief Initialize the interrupt controller (disable all, clear pending)
 */
void hal_irq_init(void);

/**
 * @brief Register a handler for a specific IRQ source
 * @param irq_num The IRQ source number (0-31)
 * @param handler Function pointer to the handler
 */
void hal_irq_register(int irq_num, void (*handler)(void));

/**
 * @brief Enable a specific IRQ source
 * @param irq_num The IRQ source number (0-31)
 */
void hal_irq_enable(int irq_num);

/**
 * @brief Disable a specific IRQ source
 * @param irq_num The IRQ source number (0-31)
 */
void hal_irq_disable(int irq_num);

/**
 * @brief Enable interrupts globally in CPSR
 */
void hal_irq_global_enable(void);

/**
 * @brief Disable interrupts globally in CPSR
 */
void hal_irq_global_disable(void);

#endif /* __HAL_IRQ_H__ */
