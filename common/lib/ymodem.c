#include "lib/ymodem.h"
#include "lib/crc16.h"
#include "hal/hal_uart.h"

#define SOH                     0x01
#define STX                     0x02
#define PACKET_HEADER           3
#define PACKET_TRAILER          2
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_1K_SIZE          1024

extern uint8_t boot_data_buf[];
static uint8_t rx_packet_buf[PACKET_1K_SIZE + PACKET_OVERHEAD];

static void print_hex8(uint8_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    hal_uart_putc(hex_chars[(val >> 4) & 0xF]);
    hal_uart_putc(hex_chars[val & 0xF]);
}

int ymodem_receive(ymodem_write_cb write_cb) {
    char c;
    int expected_len = 0;
    int received_len = 0;
    
    hal_uart_puts("\r\n[Diag] Sniffer Mode Active. Waiting for SOH/STX...\r\n");
    
    while (1) {
        hal_uart_putc(0x43); /* 'C' */
        
        if (hal_uart_getc_timeout(1000, &c) == 0) {
            if (c == SOH) {
                expected_len = 128 + PACKET_OVERHEAD;
                rx_packet_buf[0] = (uint8_t)c;
                received_len = 1;
                break;
            } else if (c == STX) {
                expected_len = PACKET_1K_SIZE + PACKET_OVERHEAD;
                rx_packet_buf[0] = (uint8_t)c;
                received_len = 1;
                break;
            }
        }
    }
    
    hal_uart_puts("\r\n[Diag] Header received! Slurping payload...\r\n");
    
    for (int i = 1; i < expected_len; i++) {
        if (hal_uart_getc_timeout(2000, &c) != 0) {
            hal_uart_puts("\r\n[Diag] FATAL: Timeout during payload slurp!\r\n");
            break;
        }
        rx_packet_buf[i] = (uint8_t)c;
        received_len++;
    }
    
    hal_uart_puts("\r\n[Diag] Slurp complete. Expected: ");
    print_hex8((expected_len >> 8) & 0xFF); print_hex8(expected_len & 0xFF);
    hal_uart_puts(", Received: ");
    print_hex8((received_len >> 8) & 0xFF); print_hex8(received_len & 0xFF);
    hal_uart_puts("\r\n");
    
    hal_uart_puts("[Diag] Packet Dump:\r\n");
    for (int i = 0; i < received_len; i++) {
        print_hex8(rx_packet_buf[i]);
        hal_uart_putc(' ');
        if ((i + 1) % 16 == 0) hal_uart_puts("\r\n");
    }
    hal_uart_puts("\r\n");
    
    if (received_len == expected_len) {
        uint32_t payload_len = expected_len - PACKET_OVERHEAD;
        uint16_t calc_crc = crc16(&rx_packet_buf[PACKET_HEADER], payload_len);
        uint16_t rx_crc = (rx_packet_buf[expected_len - 2] << 8) | rx_packet_buf[expected_len - 1];
        
        hal_uart_puts("[Diag] CRC Calc: ");
        print_hex8((calc_crc >> 8) & 0xFF); print_hex8(calc_crc & 0xFF);
        hal_uart_puts(", RX CRC: ");
        print_hex8((rx_crc >> 8) & 0xFF); print_hex8(rx_crc & 0xFF);
        hal_uart_puts("\r\n");
    }
    
    hal_uart_puts("[Diag] Sniffing finished. Aborting session.\r\n");
    hal_uart_putc(0x18); /* CAN */
    hal_uart_putc(0x18);
    
    return YMODEM_ABORT;
}
