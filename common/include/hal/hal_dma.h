/**
 * @file hal_dma.h
 * @brief Hardware Abstraction Layer for S3C2440 DMA Controller
 */

#ifndef __HAL_DMA_H__
#define __HAL_DMA_H__

#include <stdint.h>

typedef enum { DMA_CH0 = 0, DMA_CH1, DMA_CH2, DMA_CH3 } hal_dma_channel_t;

/* 定义外设/内存所挂载的总线位置 */
typedef enum {
    DMA_BUS_AHB = 0, // 系统总线 (如 SDRAM 内存)
    DMA_BUS_APB = 1  // 外设总线 (如 UART, IIC 等外设寄存器)
} hal_dma_bus_t;

/* 定义传输后的地址操作 */
typedef enum {
    DMA_ADDR_INC = 0,  // 传输后地址自动递增
    DMA_ADDR_FIXED = 1 // 传输后地址固定不变
} hal_dma_addr_mode_t;

/* 定义单次传输的数据位宽 */
typedef enum {
    DMA_DATA_BYTE = 0,      // 8-bit
    DMA_DATA_HALF_WORD = 1, // 16-bit
    DMA_DATA_WORD = 2       // 32-bit
} hal_dma_data_size_t;

/* 定义单次传输的类型 */
typedef enum {
    DMA_TRANS_UNIT = 0,  // 单次传输 (1 unit)
    DMA_TRANS_BURST4 = 1 // 突发传输 (4 units)
} hal_dma_trans_type_t;

/* DMA 配置参数结构体 */
typedef struct {
    uint32_t            src_addr;      // 源物理地址
    hal_dma_bus_t       src_bus;       // 源总线 (AHB/APB)
    hal_dma_addr_mode_t src_addr_mode; // 源地址递增/固定

    uint32_t            dst_addr;      // 目的物理地址
    hal_dma_bus_t       dst_bus;       // 目的总线 (AHB/APB)
    hal_dma_addr_mode_t dst_addr_mode; // 目的地址递增/固定

    hal_dma_data_size_t  data_size;  // 数据宽度
    hal_dma_trans_type_t trans_type; // 传输类型 (Unit/Burst4)
    uint32_t transfer_count;         // 总传输 Unit 数量 (API 会根据 Burst 模式自动调整 TC 寄存器)
} hal_dma_config_t;

/**
 * @brief  配置并初始化 DMA (软件触发模式, 适用于 Mem-to-Mem)
 */
void hal_dma_config_software(hal_dma_channel_t ch, hal_dma_config_t *cfg);

/**
 * @brief  启动 DMA 传输 (软件触发)
 */
void hal_dma_start(hal_dma_channel_t ch);

/**
 * @brief  立即停止 DMA 传输
 */
void hal_dma_stop(hal_dma_channel_t ch);

/**
 * @brief  检查当前 DMA 通道是否正在忙碌
 * @return 1 为忙碌，0 为空闲
 */
int hal_dma_is_busy(hal_dma_channel_t ch);

#endif /* __HAL_DMA_H__ */
