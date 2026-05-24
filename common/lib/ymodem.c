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

/* Global buffer to prevent stack overflow on 1KB IRQ/SVC stacks */
static uint8_t rx_packet_buf[PACKET_1K_SIZE + PACKET_OVERHEAD];

static int receive_packet(int *length, uint8_t *seq, uint32_t timeout) {
    uint16_t crc;
    char c;
    
    if (hal_uart_getc_timeout(timeout, &c) != 0) return -1;
    
    switch (c) {
        case SOH: *length = PACKET_SIZE; break;
        case STX: *length = PACKET_1K_SIZE; break;
        case EOT: return 1;
        case CAN: return 2;
        default: return -1;
    }
    
    rx_packet_buf[0] = (uint8_t)c;
    for (int i = 1; i < (*length + PACKET_OVERHEAD); i++) {
        if (hal_uart_getc_timeout(2000, &c) != 0) return -1;
        rx_packet_buf[i] = (uint8_t)c;
    }
    
    if (rx_packet_buf[PACKET_SEQNO_INDEX] != ((rx_packet_buf[PACKET_SEQNO_COMP_INDEX] ^ 0xFF) & 0xFF)) {
        return -1;
    }
    
    *seq = rx_packet_buf[PACKET_SEQNO_INDEX];
    
    crc = (uint16_t)(rx_packet_buf[*length + PACKET_HEADER] << 8);
    crc += rx_packet_buf[*length + PACKET_HEADER + 1];
    
    if (crc16(&rx_packet_buf[PACKET_HEADER], *length) != crc) {
        return -1;
    }
    
    return 0;
}

int ymodem_receive(ymodem_write_cb write_cb) {
    int session_done = 0;
    int errors = 0;
    int status;
    int length = 0;
    uint8_t seq = 0;
    uint32_t flash_offset = 0;
    uint8_t expected_seq = 0;
    int send_c = 1; 
    
    while (!session_done) {
        if (send_c) {
            hal_uart_putc(C);
            send_c = 0;
        }
        
        status = receive_packet(&length, &seq, 5000);
        
        if (status == 0) {
            errors = 0;
            
            if (seq == expected_seq) {
                if (seq == 0) {
                    /* Packet 0: Metadata */
                    if (rx_packet_buf[PACKET_HEADER] == 0) {
                        hal_uart_putc(ACK);
                        session_done = 1;
                        break;
                    }
                    hal_uart_putc(ACK);
                    send_c = 1; 
                    expected_seq = 1;
                    flash_offset = 0;
                } else {
                    /* Data Packet */
                    if (write_cb) {
                        /* Pass the pointer to the payload directly */
                        if (write_cb(flash_offset, &rx_packet_buf[PACKET_HEADER], length) != 0) {
                            hal_uart_putc(CAN);
                            hal_uart_putc(CAN);
                            return YMODEM_ERROR;
                        }
                    }
                    flash_offset += length;
                    hal_uart_putc(ACK);
                    expected_seq++;
                }
            } else if (seq == (uint8_t)(expected_seq - 1)) {
                /* Duplicate packet (our ACK was lost). Just ACK again. */
                hal_uart_putc(ACK);
            } else {
                /* Sequence error */
                hal_uart_putc(CAN);
                hal_uart_putc(CAN);
                return YMODEM_ERROR;
            }
            
        } else if (status == 1) { /* EOT */
            hal_uart_putc(NAK); 
            status = receive_packet(&length, &seq, 3000);
            if (status == 1) {
                hal_uart_putc(ACK); 
                send_c = 1;         
                expected_seq = 0;   
            } else {
                hal_uart_putc(CAN);
                hal_uart_putc(CAN);
                return YMODEM_ERROR;
            }
        } else if (status == 2) { /* CAN */
            return YMODEM_ABORT;
        } else {
            errors++;
            if (errors >= 10) return YMODEM_ERROR;
            
            if (expected_seq == 0) {
                send_c = 1;
            } else {
                hal_uart_putc(NAK);
            }
        }
    }
    return YMODEM_OK;
}
