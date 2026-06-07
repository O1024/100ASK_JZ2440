#ifndef __LL_CACHE_H__
#define __LL_CACHE_H__

#include <stdint.h>

void ll_cache_enable_icache(void);
void ll_cache_disable_icache(void);
int  ll_cache_is_icache_enabled(void);

void ll_cache_enable_dcache(void);
void ll_cache_disable_dcache(void);
int  ll_cache_is_dcache_enabled(void);

void ll_cache_clean_dcache_range(uint32_t start, uint32_t size);
void ll_cache_invalidate_dcache_range(uint32_t start, uint32_t size);

#endif /* __LL_CACHE_H__ */
