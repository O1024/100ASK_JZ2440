#include "lib/ymodem.h"
#include "hal/hal_uart.h"
#include "lib/crc16.h"
#include <stddef.h>

#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18
#define C   0x43

#define PACKET_SEQNO_INDEX      1
#define PACKET_SEQNO_COMP_INDEX 2

#define PACKET_HEADER   3
#define PACKET_TRAILER  2
#define PACKET_OVERHEAD (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE     128
#define PACKET_1K_SIZE  1024

static uint8_t rx_packet_buf[PACKET_1K_SIZE + PACKET_OVERHEAD];
static uint8_t soh_capture_buf[SOH_FRAME_SIZE + PACKET_OVERHEAD];
static int     soh_captured = 0;

static int
receive_packet(int *length, uint8_t *seq, uint32_t timeout, int *out_err, uint8_t *out_seq) {
    uint16_t crc, crc_calc;
    uint8_t  checksum, checksum_calc;
    char     c;
    int      i;
    int      err = 0;

    if (hal_uart_getc_timeout(timeout, &c) != 0) {
        return -1;
    }

    switch (c) {
        case SOH:
            *length = PACKET_SIZE;
            break;
        case STX:
            *length = PACKET_1K_SIZE;
            break;
        case EOT:
            return 1;
        case CAN:
            return 2;
        default:
            err = YMODEM_ERR_BAD_HEADER;
            goto fail;
    }

    rx_packet_buf[0] = (uint8_t)c;
    for (i = 1; i < (*length + PACKET_OVERHEAD); i++) {
        if (hal_uart_getc_timeout(5000, &c) != 0) {
            err = YMODEM_ERR_TIMEOUT;
            goto fail;
        }
        rx_packet_buf[i] = (uint8_t)c;
    }

    /* Capture SOH frame for debugging */
    if (rx_packet_buf[0] == SOH && *length == PACKET_SIZE) {
        for (i = 0; i < (PACKET_SIZE + PACKET_OVERHEAD); i++) {
            soh_capture_buf[i] = rx_packet_buf[i];
        }
        soh_captured = 1;
    }

    /* Sequence number check */
    if (rx_packet_buf[PACKET_SEQNO_INDEX] !=
        ((rx_packet_buf[PACKET_SEQNO_COMP_INDEX] ^ 0xFF) & 0xFF)) {
        err = YMODEM_ERR_SEQ;
        goto fail;
    }

    *seq = rx_packet_buf[PACKET_SEQNO_INDEX];

    /* CRC check - big endian (standard YModem CRC-16) */
    crc = (uint16_t)(rx_packet_buf[*length + PACKET_HEADER] << 8);
    crc |= rx_packet_buf[*length + PACKET_HEADER + 1];

    crc_calc = crc16(&rx_packet_buf[PACKET_HEADER], (uint32_t)(*length));

    if (crc_calc == crc) {
        return 0;
    }

    /* CRC failed - try 8-bit checksum (legacy compatibility) */
    checksum = rx_packet_buf[*length + PACKET_HEADER];
    checksum_calc = 0;
    for (i = 0; i < *length; i++) {
        checksum_calc += rx_packet_buf[PACKET_HEADER + i];
    }

    if (checksum_calc == checksum) {
        /* It's a checksum packet, accept it */
        return 0;
    }

    /* Both CRC and checksum failed */
    err = YMODEM_ERR_CRC;

fail:
    if (out_err)
        *out_err = err;
    if (out_seq)
        *out_seq = rx_packet_buf[PACKET_SEQNO_INDEX];
    return -1;
}

int ymodem_receive_ex(ymodem_write_cb write_cb,
                      ymodem_start_cb start_cb,
                      int            *out_last_error,
                      uint8_t        *out_last_seq,
                      uint32_t       *out_file_size) {
    int      session_done = 0;
    int      errors = 0;
    int      status;
    int      length = 0;
    uint8_t  seq = 0;
    uint32_t flash_offset = 0;
    uint8_t  expected_seq = 0;
    int      file_done = 0;
    int      last_err = 0;
    uint8_t  last_seq = 0;
    uint32_t file_size = 0;

    /* Initial Handshake: Send 'C' until valid packet received */
    while (1) {
        hal_uart_putc(C);
        status = receive_packet(&length, &seq, 1000, &last_err, &last_seq);
        if (status == 0) {
            break;
        }
        if (status == 2) {
            return YMODEM_ABORT;
        }
    }

    /* Main receive loop */
    while (!session_done) {
        if (status == 0) {
            errors = 0;

            if (seq == expected_seq) {
                if (seq == 0) {
                    /* Packet 0: File info header */
                    if (rx_packet_buf[PACKET_HEADER] == 0) {
                        /* Empty filename - no file to transfer */
                        hal_uart_putc(ACK);
                        session_done = 1;
                        break;
                    }
                    /* Valid filename - parse file size */
                    {
                        const uint8_t *p = &rx_packet_buf[PACKET_HEADER];
                        /* Skip filename */
                        while (*p)
                            p++;
                        p++; /* Skip null terminator */
                        /* Parse file size (decimal ASCII) */
                        file_size = 0;
                        while (*p >= '0' && *p <= '9') {
                            file_size = file_size * 10 + (*p - '0');
                            p++;
                        }
                    }
                    /* Call start callback for pre-transfer preparation */
                    if (start_cb) {
                        if (start_cb(file_size) != 0) {
                            hal_uart_putc(CAN);
                            hal_uart_putc(CAN);
                            return YMODEM_ERROR;
                        }
                    }
                    hal_uart_putc(ACK);
                    hal_uart_putc(C);
                    expected_seq = 1;
                    flash_offset = 0;
                    file_done = 0;
                } else {
                    /* Data Packet */
                    if (write_cb) {
                        if (write_cb(flash_offset,
                                     &rx_packet_buf[PACKET_HEADER],
                                     (uint32_t)length) != 0) {
                            hal_uart_putc(CAN);
                            hal_uart_putc(CAN);
                            return YMODEM_ERROR;
                        }
                    }
                    flash_offset += (uint32_t)length;
                    hal_uart_putc(ACK);
                    expected_seq++;
                }
            } else if (seq == (uint8_t)(expected_seq - 1)) {
                /* Duplicate packet - just ACK it */
                hal_uart_putc(ACK);
            } else {
                /* Unexpected sequence - abort */
                hal_uart_putc(CAN);
                hal_uart_putc(CAN);
                return YMODEM_ERROR;
            }

        } else if (status == 1) { /* EOT */
            if (!file_done) {
                /* First EOT - send NAK to request confirmation EOT */
                hal_uart_putc(NAK);
                file_done = 1;
            } else {
                /* Second EOT - transfer complete for this file */
                hal_uart_putc(ACK);
                hal_uart_putc(C);
                expected_seq = 0;
                file_done = 0;
            }

        } else if (status == 2) { /* CAN */
            return YMODEM_ABORT;

        } else { /* status == -1 (receive error) */
            errors++;
            if (errors >= 10) {
                hal_uart_putc(CAN);
                hal_uart_putc(CAN);
                return YMODEM_ERROR;
            }

            if (expected_seq == 0) {
                hal_uart_putc(C);
            } else {
                hal_uart_putc(NAK);
            }
        }

        /* Get next packet */
        status = receive_packet(&length, &seq, 3000, &last_err, &last_seq);
    }

    if (out_last_error)
        *out_last_error = last_err;
    if (out_last_seq)
        *out_last_seq = last_seq;
    if (out_file_size)
        *out_file_size = file_size;
    return YMODEM_OK;
}

int ymodem_receive(ymodem_write_cb write_cb) {
    return ymodem_receive_ex(write_cb, NULL, NULL, NULL, NULL);
}

const uint8_t *ymodem_get_last_soh(void) {
    if (!soh_captured)
        return NULL;
    return soh_capture_buf;
}

void ymodem_print_last_soh(void) {
    const uint8_t *buf = ymodem_get_last_soh();
    if (!buf) {
        hal_uart_puts("[YModem] No SOH frame captured.\r\n");
        return;
    }

    hal_uart_puts("[YModem] Last SOH frame (133 bytes):\r\n");
    for (int i = 0; i < (PACKET_SIZE + PACKET_OVERHEAD); i++) {
        uint8_t    val = buf[i];
        const char hex[] = "0123456789ABCDEF";
        hal_uart_putc(hex[(val >> 4) & 0xF]);
        hal_uart_putc(hex[val & 0xF]);
        hal_uart_putc(' ');
        if ((i & 0x0F) == 0x0F) {
            hal_uart_puts("\r\n");
        }
    }
    hal_uart_puts("\r\n");
}
