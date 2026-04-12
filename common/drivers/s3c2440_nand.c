/**
 * @file s3c2440_nand.c
 * @brief Professional S3C2440 NAND Flash Driver Implementation
 */

#include "hal/hal_nand.h"
#include "s3c2440_soc.h"

/* --- NAND Flash Command Set --- */
#define CMD_READ0       0x00
#define CMD_READ1       0x30
#define CMD_READID      0x90
#define CMD_RESET       0xFF
#define CMD_PAGEPROG1   0x80
#define CMD_PAGEPROG2   0x10
#define CMD_ERASE1      0x60
#define CMD_ERASE2      0xD0
#define CMD_READSTATUS  0x70

/* --- Helper Macros --- */
#define NAND_SELECT()   do { NAND->NFCONT &= ~(1 << 1); for(volatile int i=0; i<20; i++); } while(0)
#define NAND_DESELECT() do { NAND->NFCONT |=  (1 << 1); for(volatile int i=0; i<20; i++); } while(0)
#define NAND_WAIT()     do { \
                            for(volatile int i=0; i<50; i++); \
                            uint32_t timeout = 1000000; \
                            while (!(NAND->NFSTAT & 0x01) && timeout--); \
                        } while(0)

static void send_cmd(uint8_t cmd) {
    NAND->NFCMMD = cmd;
    for(volatile int i=0; i<10; i++);
}

static void send_addr(uint8_t addr) {
    NAND->NFADDR = addr;
    for(volatile int i=0; i<10; i++);
}

/**
 * @brief Initialize NAND Controller (NFCONF, NFCONT)
 */
void hal_nand_init(void) {
    /* Setup Timings (HCLK = 100MHz)
     * TACLS=1, TWRPH0=2, TWRPH1=1
     */
    NAND->NFCONF = (1 << 12) | (2 << 8) | (1 << 4);
    NAND->NFCONT = (1 << 4) | (1 << 1) | (1 << 0);

    /* Reset */
    NAND_SELECT();
    send_cmd(CMD_RESET);
    NAND_WAIT();
    NAND_DESELECT();
}

void hal_nand_read_id(uint8_t *id_buf) {
    NAND_SELECT();
    send_cmd(CMD_READID);
    send_addr(0x00);
    for (int i = 0; i < 5; i++) id_buf[i] = NAND->NFDATA;
    NAND_DESELECT();
}

int hal_nand_erase_block(uint32_t block_num) {
    uint32_t row = block_num * NAND_PAGES_PER_BLOCK;

    NAND_SELECT();
    send_cmd(CMD_ERASE1);
    send_addr(row & 0xFF);
    send_addr((row >> 8) & 0xFF);
    send_addr((row >> 16) & 0xFF);
    send_cmd(CMD_ERASE2);
    NAND_WAIT();

    send_cmd(CMD_READSTATUS);
    uint8_t status = NAND->NFDATA;
    NAND_DESELECT();

    return (status & 1) ? -1 : 0;
}

int hal_nand_write_page(uint32_t block, uint32_t page, const uint8_t *buffer) {
    uint32_t row = block * NAND_PAGES_PER_BLOCK + page;

    NAND_SELECT();
    send_cmd(CMD_PAGEPROG1);
    send_addr(0x00); send_addr(0x00);
    send_addr(row & 0xFF);
    send_addr((row >> 8) & 0xFF);
    send_addr((row >> 16) & 0xFF);

    for (int i = 0; i < NAND_PAGE_SIZE; i++) {
        NAND->NFDATA = buffer[i];
        for(volatile int d=0; d<5; d++);
    }

    send_cmd(CMD_PAGEPROG2);
    NAND_WAIT();
    send_cmd(CMD_READSTATUS);
    uint8_t status = NAND->NFDATA;
    NAND_DESELECT();

    return (status & 1) ? -1 : 0;
}

int hal_nand_read_page(uint32_t block, uint32_t page, uint8_t *buffer) {
    uint32_t row = block * NAND_PAGES_PER_BLOCK + page;

    NAND_SELECT();
    send_cmd(CMD_READ0);
    send_addr(0x00); send_addr(0x00);
    send_addr(row & 0xFF);
    send_addr((row >> 8) & 0xFF);
    send_addr((row >> 16) & 0xFF);
    send_cmd(CMD_READ1);
    NAND_WAIT();

    for (int i = 0; i < NAND_PAGE_SIZE; i++) {
        buffer[i] = NAND->NFDATA;
        for(volatile int d=0; d<5; d++);
    }

    NAND_DESELECT();
    return 0;
}

int hal_nand_check_bad_block(uint32_t block) {
    for (int i = 0; i < 2; i++) {
        uint32_t row = block * NAND_PAGES_PER_BLOCK + i;
        NAND_SELECT();
        send_cmd(CMD_READ0);
        send_addr(0x00); send_addr(0x08);
        send_addr(row & 0xFF);
        send_addr((row >> 8) & 0xFF);
        send_addr((row >> 16) & 0xFF);
        send_cmd(CMD_READ1);
        NAND_WAIT();
        uint8_t mark = NAND->NFDATA;
        NAND_DESELECT();
        if (mark != 0xFF) return 1;
    }
    return 0;
}
