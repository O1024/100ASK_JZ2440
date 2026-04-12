/**
 * @file hal_eth.h
 * @brief Universal Ethernet HAL Interface for JZ2440
 */

#ifndef __HAL_ETH_H__
#define __HAL_ETH_H__

#include <stdint.h>

/* --- Core API --- */

/**
 * @brief Initialize Ethernet controller in Loopback mode
 */
void hal_eth_init_loopback(void);

/**
 * @brief Send an Ethernet frame
 * @return 0 on success, -1 on failure
 */
int hal_eth_tx(const uint8_t *data, uint32_t length);

/**
 * @brief Receive an Ethernet frame
 * @return Received length, 0 if no packet, -1 on error
 */
int hal_eth_rx(uint8_t *buffer);

/**
 * @brief Print controller status for diagnostics
 */
void hal_eth_dump_status(void);

#endif /* __HAL_ETH_H__ */
