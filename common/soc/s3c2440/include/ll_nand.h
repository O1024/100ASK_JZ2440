#ifndef __LL_NAND_H__
#define __LL_NAND_H__

#include <stdint.h>

void ll_nand_init(void);
void ll_nand_read_id(uint8_t *id_buf);
int  ll_nand_erase_block(uint32_t block_num);
int  ll_nand_write_page(uint32_t block, uint32_t page, const uint8_t *buffer);
int  ll_nand_read_page(uint32_t block, uint32_t page, uint8_t *buffer);
int  ll_nand_check_bad_block(uint32_t block);

#endif /* __LL_NAND_H__ */
