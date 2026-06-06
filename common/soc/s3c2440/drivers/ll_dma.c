/**
 * @file s3c2440_dma.c
 * @brief DMA driver implementation for S3C2440 (Optimized)
 */

#include "ll_dma.h"
#include "s3c2440_soc.h"

/**
 * @brief  配置并初始化 DMA (软件触发模式, 适用于 Mem-to-Mem)
 */
void ll_dma_config_software(uint32_t ch, ll_dma_config_t *cfg) {
    dma_ch_t *dma_ch = &DMA->CH[ch];

    // 1. 设置初始源地址及控制位
    dma_ch->DISRC = cfg->src_addr;
    // LOC: 0=AHB, 1=APB; INC : 0=递增, 1=固定
    dma_ch->DISRCC = (cfg->src_bus << 1) | (cfg->src_addr_mode << 0);

    // 2. 设置初始目的地址及控制位
    dma_ch->DIDST = cfg->dst_addr;
    // CHK_INT: 0=TC达到0时中断; LOC, INC 
    dma_ch->DIDSTC = (0 << 2) | (cfg->dst_bus << 1) | (cfg->dst_addr_mode << 0);

    // 3. 配置 DCON 寄存器 (软件触发，整体服务模式)
    uint32_t dcon = 0;
    // DMD_HS: 0=需求模式(Demand mode), 1=握手模式(Handshake mode)
    // 内存到内存搬运应使用 Demand mode 以获得最高性能
    dcon |= (0U << 31);                
    dcon |= (0 << 30);                 // SYNC: 0=同步到 PCLK, 1=同步到 HCLK
    dcon |= (0 << 29);                 // INT: 0=禁止中断, 使用轮询(Polling)
    
    // TSZ: 0=单元传输(Unit), 1=突发传输(Burst4)
    dcon |= (cfg->trans_type << 28);   
    
    dcon |= (1 << 27);                 // SERVMODE: 1=整体服务模式(Whole service mode)
    dcon |= (0 << 23);                 // SWHW_SEL: 0=软件触发请求模式
    dcon |= (1 << 22);                 // RELOAD: 1=关闭自动重载
    dcon |= (cfg->data_size << 20);    // DSZ [21:20]: 传输数据宽度
    
    // TC [19:0]: 初始传输计数值
    // 如果是 Burst4 模式，TC 寄存器存的是 Burst 的次数 (TC = Units / 4)
    uint32_t tc = cfg->transfer_count;
    if (cfg->trans_type == LL_DMA_TRANS_BURST4) {
        tc /= 4;
    }
    dcon |= (tc & 0xFFFFF); 
    
    dma_ch->DCON = dcon;
}

/**
 * @brief  启动 DMA 传输 (软件触发)
 */
void ll_dma_start(uint32_t ch) {
    dma_ch_t *dma_ch = &DMA->CH[ch];
    // 先确保之前可能存在的请求被清除，然后触发
    // STOP=0, ON_OFF=1, SW_TRIG=1
    dma_ch->DMASKTRIG = (1 << 1) | (1 << 0);
}

/**
 * @brief  立即停止 DMA 传输
 */
void ll_dma_stop(uint32_t ch) {
    dma_ch_t *dma_ch = &DMA->CH[ch];
    // 配置 DMASKTRIG 的 STOP 位 = 1
    dma_ch->DMASKTRIG = (1 << 2);
}

/**
 * @brief  检查当前 DMA 通道是否正在忙碌
 * @return 1 为忙碌，0 为空闲
 */
int ll_dma_is_busy(uint32_t ch) {
    dma_ch_t *dma_ch = &DMA->CH[ch];
    // DSTAT 寄存器的 STAT [21:20]: 00=Ready, 01=Busy
    // 同时也检查 DMASKTRIG 的 ON_OFF 位，如果传输完成该位会自动清零（如果 RELOAD=1）
    return (((dma_ch->DSTAT >> 20) & 0x03) == 0x01) || (dma_ch->DMASKTRIG & (1 << 1));
}
