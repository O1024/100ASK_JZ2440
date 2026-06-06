#ifndef __LL_TIMER_H__
#define __LL_TIMER_H__

#include <stdint.h>

void     ll_timer4_init(uint32_t ms);
void     ll_timer4_start(void);
void     ll_timer4_stop(void);
void     ll_timer4_set_handler(void (*handler)(void));
void     ll_timer4_init_freerun(void);
uint16_t ll_timer4_get_ticks(void);

#endif /* __LL_TIMER_H__ */
