/**
 * @file bsp_init.h
 * @brief JZ2440 Board Support Package (BSP) Interface
 *
 * BSP 层封装板级特定的配置：
 * - 外设初始化顺序
 * - 波特率、引脚映射等板级参数
 * - 板载外设（LED、按键、LCD、以太网）配置
 */

#ifndef __BSP_INIT_H__
#define __BSP_INIT_H__

#include <stdint.h>

/* --- 板级 UART 配置 --- */
#define BSP_UART_BAUD_RATE      115200

/* --- 板载 LED GPIO --- */
#define BSP_LED1                GPF4
#define BSP_LED2                GPF5
#define BSP_LED3                GPF6

/* --- 板载按键 GPIO --- */
#define BSP_KEY1                GPF0
#define BSP_KEY2                GPF2
#define BSP_KEY3                GPG3

/* --- SDRAM 配置 --- */
#define BSP_SDRAM_SIZE          (64 * 1024 * 1024)  /* 64MB */
#define BSP_SDRAM_BASE          0x30000000

/* --- LCD 配置 --- */
#define BSP_LCD_WIDTH           480
#define BSP_LCD_HEIGHT          272
#define BSP_LCD_BPP             16
#define BSP_LCD_FB_ADDR         0x33800000

/* --- NAND 配置 --- */
#define BSP_NAND_PAGE_SIZE      2048
#define BSP_NAND_PAGES_PER_BLOCK 64
#define BSP_NAND_BLOCK_SIZE     (BSP_NAND_PAGE_SIZE * BSP_NAND_PAGES_PER_BLOCK)
#define BSP_NAND_TOTAL_SIZE     (256 * 1024 * 1024) /* 256MB */

/* --- 以太网配置 --- */
#define BSP_DM9000_BASE         0x20000000

/**
 * @brief 板级总初始化
 * 按照正确的顺序初始化所有板载外设
 */
void bsp_init(void);

/**
 * @brief 初始化系统时钟
 */
void bsp_clock_init(void);

/**
 * @brief 初始化板载 UART
 */
void bsp_uart_init(void);

/**
 * @brief 初始化板载 GPIO（LED + Key）
 */
void bsp_gpio_init(void);

/**
 * @brief 初始化板载 SDRAM
 */
void bsp_sdram_init(void);

/**
 * @brief 初始化板载 NAND 控制器
 */
void bsp_nand_init(void);

/**
 * @brief 初始化板载 LCD
 */
void bsp_lcd_init(void);

/**
 * @brief 初始化板载以太网（DM9000）
 */
void bsp_eth_init(void);

/**
 * @brief 获取系统 tick（用于超时判断）
 */
uint32_t bsp_get_tick(void);

#endif /* __BSP_INIT_H__ */
