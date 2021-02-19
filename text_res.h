#ifndef THERMOMETER_TEXT_RES_H
#define THERMOMETER_TEXT_RES_H

#include <stdint.h>

const uint8_t RES_STR_SCR_TEMP_TITLE[] = "Температура";
#define RES_TEXT(name, value) static const uint8_t RES_STR_##name[] = value;


RES_TEXT(SCR_CONSTRUCTION_MSG_LINE_0, "У процесі")
RES_TEXT(SCR_CONSTRUCTION_MSG_LINE_1, "створення")

#endif //THERMOMETER_TEXT_RES_H
