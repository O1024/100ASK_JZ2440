/**
 * @file bsp_init.c
 * @brief JZ2440 Board Support Package Implementation
 *
 * 实现板级初始化，组合 HAL 层外设驱动。
 * 所有板级特定配置（波特率、引脚、时序）集中在此。
 */

#include "bsp_init.h"
#include "hal/hal_clock.h"
#include "hal/hal_uart.h"
#include "hal/hal_gpio.h"
#include "hal/hal_sdram.h"
#include "hal/hal_nand.h"
#include "hal/hal_lcd.h"
#include "hal/hal_eth.h"

/**
 * @brief 板级总初始化
 * 顺序：时钟 -> UART -> LED -> SDRAM
 */
void bsp_init(void) {
    /* 1. 初始化系统时钟（所有外设的基础） */
    hal_clock_init();

    /* 2. 初始化串口（尽早输出调试信息） */
    bsp_uart_init();

    /* 3. 初始化板载 GPIO（LED + Key） */
    bsp_gpio_init();

    /* 4. 初始化 SDRAM（后续可能用到） */
    bsp_sdram_init();
}

/**
 * @brief 初始化系统时钟
 * JZ2440: FCLK=400MHz, HCLK=100MHz, PCLK=50MHz
 */
void bsp_clock_init(void) {
    hal_clock_init();
}

/**
 * @brief 初始化板载 UART
 * JZ2440 默认使用 UART0，波特率 115200，8N1
 */
void bsp_uart_init(void) {
    hal_uart_init(BSP_UART_BAUD_RATE);
}

/**
 * @brief 初始化板载 LED
 * JZ2440 有 3 个 LED，接在 GPF4~GPF6，低电平点亮
 */
static void bsp_led_init(void) {
    hal_gpio_init_output(BSP_LED1);
    hal_gpio_init_output(BSP_LED2);
    hal_gpio_init_output(BSP_LED3);

    /* 默认关闭所有 LED（输出高电平） */
    hal_gpio_set(BSP_LED1, GPIO_HIGH);
    hal_gpio_set(BSP_LED2, GPIO_HIGH);
    hal_gpio_set(BSP_LED3, GPIO_HIGH);
}

/**
 * @brief 初始化板载按键
 * JZ2440 有 3 个按键，接在 GPF0/GPF2/GPG3，低电平触发
 */
static void bsp_key_init(void) {
    hal_gpio_init_input(BSP_KEY1, GPIO_PULL_UP_ENABLE);
    hal_gpio_init_input(BSP_KEY2, GPIO_PULL_UP_ENABLE);
    hal_gpio_init_input(BSP_KEY3, GPIO_PULL_UP_ENABLE);
}

/**
 * @brief 初始化板载 GPIO（LED + Key）
 */
void bsp_gpio_init(void) {
    bsp_led_init();
    bsp_key_init();
}

/**
 * @brief 初始化板载 SDRAM
 * JZ2440 使用两片 HY57V561620 组成 64MB SDRAM
 */
void bsp_sdram_init(void) {
    hal_sdram_init();
}

/**
 * @brief 初始化板载 NAND 控制器
 */
void bsp_nand_init(void) {
    hal_nand_init();
}

/**
 * @brief 初始化板载 LCD
 */
void bsp_lcd_init(void) {
    hal_lcd_init();
}

/**
 * @brief 初始化板载以太网（DM9000）
 */
void bsp_eth_init(void) {
    hal_eth_init_loopback();
}

/**
 * @brief 获取系统 tick
 * 使用 Timer4 作为时基
 */
uint32_t bsp_get_tick(void) {
    /* TODO: 实现基于 Timer4 的毫秒计数 */
    return 0;
}
