#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"

/**
 * @brief 简单的循环延时函数
 */
void delay(volatile int d) {
    while (d--);
}

int main(void) {
    // 1. 初始化系统时钟为 400MHz (这是与 01_led_chaser 唯一不同的地方)
    hal_clock_init();

    // 2. 初始化 GPF4, GPF5, GPF6 为输出模式 (参考 01_led_chaser)
    hal_gpio_init_output(GPF4);
    hal_gpio_init_output(GPF5);
    hal_gpio_init_output(GPF6);

    // 3. 初始状态：全部熄灭 (高电平)
    hal_gpio_set(GPF4, GPIO_HIGH);
    hal_gpio_set(GPF5, GPIO_HIGH);
    hal_gpio_set(GPF6, GPIO_HIGH);

    while (1) {
        // LED 4 亮
        hal_gpio_set(GPF4, GPIO_LOW);
        delay(250000);
        hal_gpio_set(GPF4, GPIO_HIGH);

        // LED 5 亮
        hal_gpio_set(GPF5, GPIO_LOW);
        delay(250000);
        hal_gpio_set(GPF5, GPIO_HIGH);

        // LED 6 亮
        hal_gpio_set(GPF6, GPIO_LOW);
        delay(250000);
        hal_gpio_set(GPF6, GPIO_HIGH);
    }

    return 0;
}
