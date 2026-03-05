#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"

/**
 * @brief 简单的循环延时函数
 */
void delay(volatile int d) {
    while (d--);
}

/**
 * @brief 带消抖的按键读取函数
 * @return GPIO_LOW 表示确认按下，GPIO_HIGH 表示未按下
 */
hal_gpio_state_t get_key_debounced(hal_gpio_pin_t pin) {
    if (hal_gpio_get(pin) == GPIO_LOW) {
        delay(100000); // 软件延时消抖 (约 5-10ms @ 400MHz)
        if (hal_gpio_get(pin) == GPIO_LOW) {
            return GPIO_LOW;
        }
    }
    return GPIO_HIGH;
}

int main(void) {
    /* 0. 禁用看门狗 (防止程序未从 start.o 启动导致不停复位) */
    /* S3C2440 WTCON 寄存器地址为 0x53000000 */
    *((volatile unsigned int *)0x53000000) = 0;

    /* 1. 初始化系统时钟 (400MHz) */
    hal_clock_init();

    /* 2. 初始化 LED (输出) */
    hal_gpio_init_output(GPF4);
    hal_gpio_init_output(GPF5);
    hal_gpio_init_output(GPF6);

    /* 3. 初始化按键 (输入) */
    hal_gpio_init_input(GPF0);
    hal_gpio_init_input(GPF2);

    /* 4. 初始状态: LED 全灭 (高电平) */
    hal_gpio_set(GPF4, GPIO_HIGH);
    hal_gpio_set(GPF5, GPIO_HIGH);
    hal_gpio_set(GPF6, GPIO_HIGH);

    while (1) {
        /* 读取 S2 (GPF0) 控制 LED1 (GPF4) */
        if (get_key_debounced(GPF0) == GPIO_LOW) {
            hal_gpio_set(GPF4, GPIO_LOW);   // 点亮
        } else {
            hal_gpio_set(GPF4, GPIO_HIGH);  // 熄灭
        }

        /* 读取 S3 (GPF2) 控制 LED2 (GPF5) */
        if (get_key_debounced(GPF2) == GPIO_LOW) {
            hal_gpio_set(GPF5, GPIO_LOW);   // 点亮
        } else {
            hal_gpio_set(GPF5, GPIO_HIGH);  // 熄灭
        }

        /* LED3 (GPF6) 默认熄灭 */
        hal_gpio_set(GPF6, GPIO_HIGH);
    }

    return 0;
}
