#include "lib/crc16.h"

uint16_t crc16(const uint8_t *buf, uint32_t len) {
    uint16_t crc = 0;
    while (len--) {
        crc ^= *buf++ << 8;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}
