#ifndef __LL_ETH_H__
#define __LL_ETH_H__

#include <stdint.h>

void ll_eth_init_loopback(void);
int  ll_eth_tx(const uint8_t *data, uint32_t length);
int  ll_eth_rx(uint8_t *buffer);
void ll_eth_dump_status(void);

#endif /* __LL_ETH_H__ */
