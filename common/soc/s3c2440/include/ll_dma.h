#ifndef __LL_DMA_H__
#define __LL_DMA_H__

#include <stdint.h>

typedef struct {
    uint32_t src_addr;
    uint32_t src_bus;
    uint32_t src_addr_mode;

    uint32_t dst_addr;
    uint32_t dst_bus;
    uint32_t dst_addr_mode;

    uint32_t data_size;
    uint32_t trans_type; /* 0=unit, 1=burst4 */
    uint32_t transfer_count;
} ll_dma_config_t;

#define LL_DMA_TRANS_UNIT   0
#define LL_DMA_TRANS_BURST4 1

void ll_dma_config_software(uint32_t ch, ll_dma_config_t *cfg);
void ll_dma_start(uint32_t ch);
void ll_dma_stop(uint32_t ch);
int  ll_dma_is_busy(uint32_t ch);

#endif /* __LL_DMA_H__ */
