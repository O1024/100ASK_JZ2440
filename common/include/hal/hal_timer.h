/**
 * @file hal_timer.h
 * @brief Hardware Abstraction Layer for S3C2440 PWM Timers
 */

#ifndef __HAL_TIMER_H__
#define __HAL_TIMER_H__

#include <stdint.h>

/**
 * @brief Initialize Timer 4 for periodic interrupts
 * @param ms Interval in milliseconds
 */
void hal_timer4_init(uint32_t ms);

/**
 * @brief Start Timer 4
 */
void hal_timer4_start(void);

/**
 * @brief Stop Timer 4
 */
void hal_timer4_stop(void);

/**
 * @brief Set the handler for Timer 4 interrupt
 * @param handler Function pointer to the handler
 */
void hal_timer4_set_handler(void (*handler)(void));

/**
 * @brief Initialize Timer 4 as a free-running down-counter (1 tick ~= 32us)
 * Useful for hardware timeouts without interrupts.
 */
void hal_timer4_init_freerun(void);

/**
 * @brief Get the current value of the Timer 4 observation register
 * @return Current tick value (down-counting)
 */
uint16_t hal_timer4_get_ticks(void);

#endif /* __HAL_TIMER_H__ */
