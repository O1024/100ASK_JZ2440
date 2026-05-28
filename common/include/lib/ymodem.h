#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>

/* YModem status codes */
#define YMODEM_OK       0
#define YMODEM_ERROR   -1
#define YMODEM_ABORT   -2
#define YMODEM_TIMEOUT -3

/* Error codes for detailed debugging */
#define YMODEM_ERR_BAD_HEADER   1  /* First byte not SOH/STX/EOT/CAN */
#define YMODEM_ERR_TIMEOUT      2  /* Timeout reading packet body */
#define YMODEM_ERR_SEQ          3  /* Sequence number mismatch */
#define YMODEM_ERR_CRC          4  /* CRC/Checksum mismatch */

/* SOH frame capture for debugging */
#define SOH_FRAME_SIZE  128

/* Callback type for writing received data chunk */
typedef int (*ymodem_write_cb)(uint32_t offset, const uint8_t *data, uint32_t length);

/* Callback type for pre-transfer preparation (e.g. erase flash blocks) */
typedef int (*ymodem_start_cb)(uint32_t file_size);

/**
 * @brief Receive a file via YModem protocol
 * @param write_cb Callback function to handle received data
 * @return Status code (YMODEM_OK on success)
 */
int ymodem_receive(ymodem_write_cb write_cb);

/**
 * @brief Receive a file via YModem protocol with detailed error info
 * @param write_cb  Callback function to handle received data
 * @param start_cb  Optional callback called after file header is parsed (for preparation like erasing flash)
 * @param out_last_error Output: last packet error code (YMODEM_ERR_*)
 * @param out_last_seq   Output: last packet sequence number
 * @param out_file_size  Output: file size from YModem header (0 if unknown)
 * @return Status code (YMODEM_OK on success)
 */
int ymodem_receive_ex(ymodem_write_cb write_cb, ymodem_start_cb start_cb, int *out_last_error, uint8_t *out_last_seq, uint32_t *out_file_size);

/**
 * @brief Get the last captured SOH frame (for debugging)
 * @return Pointer to 128-byte SOH frame buffer, or NULL if no SOH received
 */
const uint8_t *ymodem_get_last_soh(void);

/**
 * @brief Print the last captured SOH frame in hex (for debugging)
 */
void ymodem_print_last_soh(void);

#endif
