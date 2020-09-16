#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define OW_SINGLE_BYTE(value)   (const uint8_t[]) { (value) }, 1
#define OW_READ_SLOTS(count)    NULL, count

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

// Pass [data] as NULL to put RX slots
void ow_txbuf_put(const uint8_t *data, uint32_t byte_len);

void ow_rxbuf_get(uint8_t *data, uint32_t byte_len);

#endif
