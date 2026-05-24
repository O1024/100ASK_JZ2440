#ifndef __CRC16_H__
#define __CRC16_H__

#include <stdint.h>

/**
 * @brief Calculate CRC16 (XMODEM/YMODEM)
 * @param buf Data buffer
 * @param len Data length
 * @return CRC16 value
 */
uint16_t crc16(const uint8_t *buf, uint32_t len);

#endif
