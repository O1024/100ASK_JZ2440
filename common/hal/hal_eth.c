#include "hal/hal_eth.h"
#include "ll_eth.h"

void hal_eth_init_loopback(void) {
    ll_eth_init_loopback();
}

int hal_eth_tx(const uint8_t *data, uint32_t length) {
    return ll_eth_tx(data, (uint16_t)length);
}

int hal_eth_rx(uint8_t *buffer) {
    return ll_eth_rx(buffer);
}

void hal_eth_dump_status(void) {
    ll_eth_dump_status();
}
