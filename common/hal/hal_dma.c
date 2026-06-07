#include "hal/hal_dma.h"
#include "ll_dma.h"

void hal_dma_config_software(hal_dma_channel_t ch, hal_dma_config_t *cfg) {
    ll_dma_config_t ll_cfg;
    ll_cfg.src_addr = cfg->src_addr;
    ll_cfg.src_bus = cfg->src_bus;
    ll_cfg.src_addr_mode = cfg->src_addr_mode;
    ll_cfg.dst_addr = cfg->dst_addr;
    ll_cfg.dst_bus = cfg->dst_bus;
    ll_cfg.dst_addr_mode = cfg->dst_addr_mode;
    ll_cfg.data_size = cfg->data_size;
    ll_cfg.trans_type =
        (cfg->trans_type == DMA_TRANS_BURST4) ? LL_DMA_TRANS_BURST4 : LL_DMA_TRANS_UNIT;
    ll_cfg.transfer_count = cfg->transfer_count;
    ll_dma_config_software(ch, &ll_cfg);
}

void hal_dma_start(hal_dma_channel_t ch) {
    ll_dma_start(ch);
}

void hal_dma_stop(hal_dma_channel_t ch) {
    ll_dma_stop(ch);
}

int hal_dma_is_busy(hal_dma_channel_t ch) {
    return ll_dma_is_busy(ch);
}
