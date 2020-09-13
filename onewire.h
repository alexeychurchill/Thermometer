#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>

void ow_init();

void ow_reset();

uint32_t ow_is_running();

uint32_t ow_get_no_devices();

void ow_start_transceiver(uint16_t byte_len);

void ow_txbuf_put_byte(uint8_t data);

void ow_txbuf_put_bytes(uint8_t *data, uint32_t byte_len);

void ow_txbuf_put_rx_slots(uint32_t byte_len);

uint32_t ow_rxbuf_is_presence_ok();

uint8_t ow_rxbuf_get_byte();

void ow_rxbuf_get_bytes(uint8_t *data, uint32_t byte_len);

#endif
