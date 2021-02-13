#ifndef THERMOMETER_TEXT_RES_H
#define THERMOMETER_TEXT_RES_H

#include <stdint.h>

const uint8_t RES_STR_SCR_TEMP_TITLE[] = "Температура";
#define RES_TEXT(name, value) static const uint8_t RES_STR_##name[] = value;


#endif //THERMOMETER_TEXT_RES_H
