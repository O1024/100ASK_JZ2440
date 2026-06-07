/**
 * @file hal_nand.c
 * @brief NAND Flash HAL Implementation
 *
 * Generic logic layer. Calls LLD primitives.
 */

#include "hal/hal_nand.h"
#include "ll_nand.h"

void hal_nand_init(void) {
    ll_nand_init();
}

void hal_nand_read_id(uint8_t *id_buf) {
    ll_nand_read_id(id_buf);
}

int hal_nand_erase_block(uint32_t block_num) {
    return ll_nand_erase_block(block_num);
}

int hal_nand_write_page(uint32_t block, uint32_t page, const uint8_t *buffer) {
    return ll_nand_write_page(block, page, buffer);
}

int hal_nand_read_page(uint32_t block, uint32_t page, uint8_t *buffer) {
    return ll_nand_read_page(block, page, buffer);
}

int hal_nand_check_bad_block(uint32_t block) {
    return ll_nand_check_bad_block(block);
}

/**
 * @brief High-level NAND read supporting multi-page cross-boundary access
 *
 * Used by SPL for relocation and by App for data loading.
 */
int hal_nand_read(uint8_t *dest, uint32_t src_offset, uint32_t size) {
    uint32_t page_idx = src_offset / NAND_PAGE_SIZE;
    uint32_t current_size = 0;

    while (current_size < size) {
        if (ll_nand_read_page(page_idx / NAND_PAGES_PER_BLOCK,
                              page_idx % NAND_PAGES_PER_BLOCK,
                              dest + current_size) != 0) {
            return -1;
        }
        current_size += NAND_PAGE_SIZE;
        page_idx++;
    }
    return 0;
}

int hal_nand_write(const uint8_t *src, uint32_t dest_offset, uint32_t size) {
    uint32_t page_idx = dest_offset / NAND_PAGE_SIZE;
    uint32_t current_size = 0;

    while (current_size < size) {
        if (ll_nand_write_page(page_idx / NAND_PAGES_PER_BLOCK,
                               page_idx % NAND_PAGES_PER_BLOCK,
                               src + current_size) != 0) {
            return -1;
        }
        current_size += NAND_PAGE_SIZE;
        page_idx++;
    }
    return 0;
}

int hal_nand_erase(uint32_t offset) {
    uint32_t block_num = offset / NAND_BLOCK_SIZE;
    return ll_nand_erase_block(block_num);
}
