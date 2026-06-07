#include "hal/hal_cache.h"
#include "ll_cache.h"

void hal_cache_enable_icache(void) {
    ll_cache_enable_icache();
}

void hal_cache_disable_icache(void) {
    ll_cache_disable_icache();
}

int hal_cache_is_icache_enabled(void) {
    return ll_cache_is_icache_enabled();
}

void hal_cache_enable_dcache(void) {
    ll_cache_enable_dcache();
}

void hal_cache_disable_dcache(void) {
    ll_cache_disable_dcache();
}

int hal_cache_is_dcache_enabled(void) {
    return ll_cache_is_dcache_enabled();
}
