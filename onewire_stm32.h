#ifndef ONEWIRE_STM32_H
#define ONEWIRE_STM32_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "stm32f1xx.h"
#include "onewire.h"

#define OW_SINGLE_BYTE(value)   (const uint8_t[]) { (value) }, 1
#define OW_READ_SLOTS(count)    NULL, count

void ow_start_bus();

bool ow_is_busy();

OwOperation_t ow_get_operation();

OwError_t ow_get_error();

void ow_send_reset();

void ow_start_rxtx();

/**
 * Puts data / read slots to the transmission buffer
 *
 * @param data array of data that should be transmitted
 *        OR NULL to put read slots
 * @param byte_len length of the data (or, read slots) in bytes
 * @param append false if buffer should be overwritten
 */
void ow_txbuf_put(const uint8_t *data, uint32_t byte_len, bool append);

void ow_rxbuf_get(uint8_t *data, uint32_t byte_len);

#endif
