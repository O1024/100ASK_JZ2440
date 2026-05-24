/**
 * @file hal_nand.h
 * @brief Professional NAND Flash HAL Interface for JZ2440
 */

#ifndef __HAL_NAND_H__
#define __HAL_NAND_H__

#include <stdint.h>

/* --- NAND Geometry (K9F2G08U0M 256MB) --- */
#define NAND_PAGE_SIZE          2048
#define NAND_SPARE_SIZE         64
#define NAND_PAGE_TOTAL_SIZE    (NAND_PAGE_SIZE + NAND_SPARE_SIZE)
#define NAND_PAGES_PER_BLOCK    64
#define NAND_BLOCK_SIZE         (NAND_PAGE_SIZE * NAND_PAGES_PER_BLOCK)
#define NAND_BLOCK_COUNT        2048
#define NAND_TOTAL_SIZE         (NAND_BLOCK_SIZE * NAND_BLOCK_COUNT)

/* --- Status Definitions --- */
#define NAND_STATUS_READY       0x40
#define NAND_STATUS_ERROR       0x01

/* --- Core API --- */
void hal_nand_init(void);
void hal_nand_read_id(uint8_t *id_buf);
int  hal_nand_read(uint8_t *dest, uint32_t src_offset, uint32_t size);
int  hal_nand_write(const uint8_t *src, uint32_t dest_offset, uint32_t size);
int  hal_nand_erase(uint32_t offset);

int  hal_nand_erase_block(uint32_t block_num);
int  hal_nand_write_page(uint32_t block, uint32_t page, const uint8_t *buffer);
int  hal_nand_read_page(uint32_t block, uint32_t page, uint8_t *buffer);
int  hal_nand_check_bad_block(uint32_t block);

#endif /* __HAL_NAND_H__ */
