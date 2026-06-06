#ifndef __LL_CLOCK_H__
#define __LL_CLOCK_H__

#include <stdint.h>

void     ll_clock_init(void);
void     ll_clock_reset(void);
uint32_t ll_clock_get_pclk(void);

#endif /* __LL_CLOCK_H__ */
