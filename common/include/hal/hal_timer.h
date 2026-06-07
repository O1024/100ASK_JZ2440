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

/**
 * @brief Reset the Timer4 overflow counter to zero.
 * Call this right before starting a measurement.
 */
void hal_timer4_reset_overflows(void);

/**
 * @brief Get the number of Timer4 overflows since last reset.
 * @return Overflow count (each overflow = 65536 ticks ≈ 2.097 s)
 */
uint32_t hal_timer4_get_overflows(void);

/**
 * @brief Calculate elapsed ticks between two Timer4 readings.
 *
 * This function correctly handles 16-bit counter wrap-around and
 * accumulated overflows, allowing measurements longer than 2.1 seconds.
 *
 * @param start_ticks Value from hal_timer4_get_ticks() at start
 * @param end_ticks   Value from hal_timer4_get_ticks() at end
 * @return Total elapsed ticks (32-bit, supports long durations)
 */
uint32_t hal_timer4_get_elapsed_ticks(uint16_t start_ticks, uint16_t end_ticks);

/**
 * @brief Convert ticks to microseconds.
 * @param ticks Number of timer ticks
 * @return Approximate time in microseconds (1 tick ≈ 32 us)
 */
uint32_t hal_timer4_ticks_to_us(uint32_t ticks);

#endif /* __HAL_TIMER_H__ */
