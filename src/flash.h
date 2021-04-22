#ifndef THERMOMETER_FLASH_H
#define THERMOMETER_FLASH_H
#include <stdint.h>
#include <stdbool.h>

void flash_unlock();

void flash_lock();

void flash_erase(uint32_t page_address);

bool flash_write(uint32_t address, const uint32_t *data, uint32_t size);

#endif //THERMOMETER_FLASH_H
