#ifndef __LL_TIMER_H__
#define __LL_TIMER_H__

#include <stdint.h>

void     ll_timer4_init(uint32_t ms);
void     ll_timer4_start(void);
void     ll_timer4_stop(void);
void     ll_timer4_set_handler(void (*handler)(void));
void     ll_timer4_init_freerun(void);
uint16_t ll_timer4_get_ticks(void);

/**
 * @brief Reset the Timer4 overflow counter to zero.
 * Call this right before starting a measurement.
 */
void     ll_timer4_reset_overflows(void);

/**
 * @brief Get the number of Timer4 overflows since reset.
 * @return Overflow count (each overflow = 65536 ticks)
 */
uint32_t ll_timer4_get_overflows(void);

#endif /* __LL_TIMER_H__ */
