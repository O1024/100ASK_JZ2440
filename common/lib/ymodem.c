#include "lib/ymodem.h"
#include "lib/crc16.h"
#include "hal/hal_uart.h"

#define SOH                     0x01
#define STX                     0x02
#define EOT                     0x04
#define ACK                     0x06
#define NAK                     0x15
#define CAN                     0x18
#define C                       0x43

#define PACKET_SEQNO_INDEX      1
#define PACKET_SEQNO_COMP_INDEX 2

#define PACKET_HEADER           3
#define PACKET_TRAILER          2
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             128
#define PACKET_1K_SIZE          1024

static int receive_packet(uint8_t *data, int *length, uint32_t timeout) {
    uint16_t crc;
    uint8_t packet_size[PACKET_1K_SIZE + PACKET_OVERHEAD];
    char c;
    
    *length = 0;
    if (hal_uart_getc_timeout(timeout, &c) != 0) return -1;
    
    switch (c) {
        case SOH: *length = PACKET_SIZE; break;
        case STX: *length = PACKET_1K_SIZE; break;
        case EOT: return 1;
        case CAN: return 2;
        default: return -1;
    }
    
    packet_size[0] = c;
    for (int i = 1; i < (*length + PACKET_OVERHEAD); i++) {
        if (hal_uart_getc_timeout(1000, &c) != 0) return -1;
        packet_size[i] = c;
    }
    
    if (packet_size[PACKET_SEQNO_INDEX] != ((packet_size[PACKET_SEQNO_COMP_INDEX] ^ 0xFF) & 0xFF)) {
        return -1;
    }
    
    crc = (packet_size[*length + PACKET_HEADER] << 8);
    crc += packet_size[*length + PACKET_HEADER + 1];
    
    if (crc16(&packet_size[PACKET_HEADER], *length) != crc) {
        return -1;
    }
    
    for (int i = 0; i < *length; i++) {
        data[i] = packet_size[PACKET_HEADER + i];
    }
    
    return 0;
}

int ymodem_receive(ymodem_write_cb write_cb) {
    uint8_t packet_data[PACKET_1K_SIZE];
    uint8_t file_done = 0;
    int session_done = 0;
    int errors = 0;
    int status;
    int length = 0;
    uint32_t offset = 0;
    
    while (!session_done) {
        hal_uart_putc(C);
        
        status = receive_packet(packet_data, &length, 3000);
        if (status == 0) {
            /* Packet 0: Filename (ignore and ACK) */
            if (packet_data[0] == 0) {
                hal_uart_putc(ACK);
                session_done = 1;
                break;
            }
            errors = 0;
            if (write_cb) {
                if (write_cb(offset, packet_data, length) != 0) {
                    hal_uart_putc(CAN);
                    hal_uart_putc(CAN);
                    return YMODEM_ERROR;
                }
            }
            offset += length;
            hal_uart_putc(ACK);
        } else if (status == 1) { /* EOT */
            hal_uart_putc(ACK);
            file_done = 1;
        } else if (status == 2) { /* CAN */
            return YMODEM_ABORT;
        } else {
            hal_uart_putc(NAK);
            errors++;
            if (errors >= 5) return YMODEM_ERROR;
        }
        
        if (file_done) {
            /* YModem sends a second C to get the next file (or null packet to end) */
            hal_uart_putc(C);
            status = receive_packet(packet_data, &length, 3000);
            if (status == 0 && packet_data[0] == 0) {
                hal_uart_putc(ACK);
                session_done = 1;
            } else {
                hal_uart_putc(CAN);
                hal_uart_putc(CAN);
                return YMODEM_ERROR;
            }
        }
    }
    return YMODEM_OK;
}
