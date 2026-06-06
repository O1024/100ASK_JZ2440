/**
 * @file s3c2440_eth.c
 * @brief Industrial-grade Ethernet Driver for S3C2440 (DM9000C)
 */

#include "ll_eth.h"
#include "ll_uart.h"
#include "s3c2440_soc.h"

/* --- Hardware Access --- */
#define ETH_IO_BASE     0x20000000
#define ETH_INDEX       (*(volatile uint16_t *)(ETH_IO_BASE + 0x000))
#define ETH_DATA        (*(volatile uint16_t *)(ETH_IO_BASE + 0x004))

/* --- DM9000 Registers --- */
#define REG_NCR         0x00    /* Network Control */
#define REG_NSR         0x01    /* Network Status */
#define REG_TCR         0x02    /* TX Control */
#define REG_RCR         0x05    /* RX Control */
#define REG_RSR         0x06    /* RX Status */
#define REG_GPR         0x1F    /* GPIO Write (PHY Control) */
#define REG_PAR         0x10    /* Physical Address (MAC) */
#define REG_MRRH        0xF5    /* RX Read Pointer High */
#define REG_MRRL        0xF4    /* RX Read Pointer Low */
#define REG_MWRH        0xFB    /* Memory Write Pointer High */
#define REG_MWRL        0xFA    /* Memory Write Pointer Low */
#define REG_TXPLL       0xFC    /* TX Packet Length Low */
#define REG_TXPLH       0xFD    /* TX Packet Length High */
#define REG_ISR         0xFE    /* Interrupt Status */
#define REG_IMR         0xFF    /* Interrupt Mask */
#define REG_MRCMDX      0xF0    /* Memory Read CMD (No Increment) */
#define REG_MRCMD       0xF2    /* Memory Read CMD (Increment) */
#define REG_MWCMD       0xF8    /* Memory Write CMD (Increment) */

/* Bits & Values */
#define NCR_RST         (1 << 0)
#define NCR_LBK_MAC     (1 << 1)
#define TCR_TXREQ       (1 << 0)
#define RCR_RXEN        (1 << 0)
#define RCR_PRMSC       (1 << 1)
#define RCR_RUNT        (1 << 2)
#define RSR_ERR_BITS    ((1<<7)|(1<<2)|(1<<1)) /* Runt, Alignment, CRC errors */

static uint8_t eth_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

/* --- Internal Helpers --- */

static void eth_write_reg(uint8_t reg, uint8_t data) {
    ETH_INDEX = reg;
    for (volatile int i = 0; i < 20; i++);
    ETH_DATA = data;
}

static uint8_t eth_read_reg(uint8_t reg) {
    ETH_INDEX = reg;
    for (volatile int i = 0; i < 20; i++);
    return (uint8_t)(ETH_DATA & 0xFF);
}

/**
 * @brief Correctly reset internal SRAM pointers
 * TX: 0x0000 ~ 0x0BFF
 * RX: 0x0C00 ~ 0x3FFF
 */
static void eth_reset_pointers(void) {
    /* Set TX Write Pointer to 0x0000 */
    eth_write_reg(REG_MWRH, 0x00);
    eth_write_reg(REG_MWRL, 0x00);
    /* Set RX Read Pointer to 0x0C00 */
    eth_write_reg(REG_MRRH, 0x0C);
    eth_write_reg(REG_MRRL, 0x00);
}

/* --- Public API --- */

void ll_eth_init_loopback(void) {
    /* 1. Bus Configuration */
    MEM_CTL->BWSCON &= ~(0xF << 16);
    MEM_CTL->BWSCON |=  (0x1 << 16); 
    MEM_CTL->BANKCON[4] = 0x00007FF0; 

    /* 2. Hardware Reset Sequence */
    eth_write_reg(REG_GPR, 0x00); /* Power up internal PHY */
    for (volatile int i = 0; i < 10000; i++);
    eth_write_reg(REG_NCR, NCR_RST);
    for (volatile int i = 0; i < 10000; i++);
    eth_write_reg(REG_NCR, 0);

    /* 3. MAC & Loopback Configuration (Bit 3 is RO, omitted) */
    eth_write_reg(REG_NCR, NCR_LBK_MAC);
    for (int i = 0; i < 6; i++) eth_write_reg(REG_PAR + i, eth_mac[i]);

    /* 4. Operation Modes */
    eth_write_reg(REG_RCR, RCR_RXEN | RCR_PRMSC | RCR_RUNT | (1 << 3));
    eth_reset_pointers();
    eth_write_reg(REG_ISR, 0xFF);
    eth_write_reg(REG_IMR, 0x80); /* PAR Enable for auto-wrapping */
}

int ll_eth_tx(const uint8_t *data, uint32_t length) {
    uint32_t total_len = 14 + length;
    uint8_t header[14];
    for (int i = 0; i < 6; i++) header[i] = 0xFF;
    for (int i = 0; i < 6; i++) header[i + 6] = eth_mac[i];
    header[12] = 0x08; header[13] = 0x00;

    /* Write data to TX SRAM */
    ETH_INDEX = REG_MWCMD;
    for (int i = 0; i < 14; i += 2) ETH_DATA = (header[i] | (header[i+1] << 8));
    for (int i = 0; i < length; i += 2) {
        uint16_t val = (i + 1 < length) ? (data[i] | (data[i+1] << 8)) : data[i];
        ETH_DATA = val;
    }

    /* Set length and trigger TX */
    eth_write_reg(REG_TXPLL, total_len & 0xFF);
    eth_write_reg(REG_TXPLH, (total_len >> 8) & 0xFF);
    eth_write_reg(REG_TCR, TCR_TXREQ);

    /* [FIX 1] Poll TCR_TXREQ instead of NSR bits to support Ping-Pong buffers */
    uint32_t timeout = 1000000;
    while ((eth_read_reg(REG_TCR) & TCR_TXREQ) && timeout--);
    
    if (timeout == 0) return -1;
    return 0;
}

int ll_eth_rx(uint8_t *buffer) {
    /* 1. Check packet ready flag without incrementing pointer */
    if (!(eth_read_reg(REG_MRCMDX) & 0x01)) return 0;

    /* 2. Read Packet Header (Status Word + Length Word) */
    ETH_INDEX = REG_MRCMD;
    for (volatile int i = 0; i < 20; i++);
    
    uint16_t header_word = ETH_DATA; /* [Status (RSR)] [Flag (01h)] */
    uint16_t length_word = ETH_DATA; /* [Length] */

    uint8_t flag   = header_word & 0xFF;
    uint8_t status = (header_word >> 8) & 0xFF;
    uint16_t length = length_word;

    /* [FIX 3 & 4] Handle sync errors and bad packets via dummy reads */
    if (flag != 0x01 || (status & RSR_ERR_BITS) || length > 1536) {
        /* Dummy read to advance hardware pointer if length is sane */
        if (length > 0 && length <= 1536) {
            for (int i = 0; i < length; i += 2) (void)ETH_DATA;
        } else {
            /* Fatal sync error, requires re-init or complex recovery */
            eth_reset_pointers();
        }
        return -1;
    }

    /* 3. Valid Data Read */
    for (int i = 0; i < length; i += 2) {
        uint16_t val = ETH_DATA;
        buffer[i] = val & 0xFF;
        if (i + 1 < length) buffer[i + 1] = (val >> 8) & 0xFF;
    }

    return (int)length;
}

void ll_eth_dump_status(void) {
    /* LLD layer does not output debug info.
     * HAL layer can read registers via ll_eth and format output.
     */
}
