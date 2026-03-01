#include "hal/hal_gpio.h"

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
        delay(1000); // 软件延时消抖 (约 5-10ms，取决于主频)
        if (hal_gpio_get(pin) == GPIO_LOW) {
            return GPIO_LOW;
        }
    }
    return GPIO_HIGH;
}

int main(void) {
    /* 1. 初始化 LED (输出) */
    hal_gpio_init_output(GPF4);
    hal_gpio_init_output(GPF5);
    hal_gpio_init_output(GPF6);

    /* 2. 初始化按键 (输入) */
    hal_gpio_init_input(GPF0);
    hal_gpio_init_input(GPF2);

    /* 3. 初始状态: LED 全灭 (高电平) */
    hal_gpio_set(GPF4, GPIO_HIGH);
    hal_gpio_set(GPF5, GPIO_HIGH);
    hal_gpio_set(GPF6, GPIO_HIGH);

    while (1) {
        /* 读取 S2 (GPF0) 控制 LED4 */
        if (get_key_debounced(GPF0) == GPIO_LOW) {
            hal_gpio_set(GPF4, GPIO_LOW);   // 点亮
        } else {
            hal_gpio_set(GPF4, GPIO_HIGH);  // 熄灭
        }

        /* 读取 S3 (GPF2) 控制 LED5 */
        if (get_key_debounced(GPF2) == GPIO_LOW) {
            hal_gpio_set(GPF5, GPIO_LOW);   // 点亮
        } else {
            hal_gpio_set(GPF5, GPIO_HIGH);  // 熄灭
        }
    }

    return 0;
}
