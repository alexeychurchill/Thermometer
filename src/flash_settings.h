#ifndef THERMOMETER_FLASH_SETTINGS_H
#define THERMOMETER_FLASH_SETTINGS_H
#include <stdint.h>
#include <stdbool.h>

void settings_init();

uint32_t settings_get_sleep();

bool settings_set_sleep(uint32_t value);

#endif //THERMOMETER_FLASH_SETTINGS_H
