#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>
#include <stdbool.h>

#define OW_SINGLE_BYTE(value) (const uint8_t[]) { (value) }, 1

typedef enum OwError {
    OW_ERROR_NONE,
    OW_ERROR_NO_DEVICES,
    OW_ERROR_PRESENCE_WRONG_LENGTH
} OwError_t;

void ow_start();

void ow_reset();

uint32_t ow_is_busy();

OwError_t ow_get_error();

void ow_start_transceiver(uint16_t byte_len, bool wait_done);

void ow_txbuf_put_bytes(const uint8_t *data, uint32_t byte_len);

void ow_txbuf_put_rx_slots(uint32_t byte_len);

void ow_rxbuf_get_bytes(uint8_t *data, uint32_t byte_len);

#endif
