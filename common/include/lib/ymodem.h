#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>

/* YModem status codes */
#define YMODEM_OK       0
#define YMODEM_ERROR   -1
#define YMODEM_ABORT   -2
#define YMODEM_TIMEOUT -3

/* Callback type for writing received data chunk */
typedef int (*ymodem_write_cb)(uint32_t offset, const uint8_t *data, uint32_t length);

/**
 * @brief Receive a file via YModem protocol
 * @param write_cb Callback function to handle received data
 * @return Status code (YMODEM_OK on success)
 */
int ymodem_receive(ymodem_write_cb write_cb);

#endif
