#ifndef THERMOMETER_TEXT_RES_H
#define THERMOMETER_TEXT_RES_H

#include <stdint.h>

#define RES_TEXT(name, value) static const uint8_t RES_STR_##name[] = value;

static const uint8_t RES_STR_SCR_TEMP_TITLE[] = "Температура";

RES_TEXT(SCR_INIT_CAPTION, "Привіт")

RES_TEXT(SCR_MENU_TITLE, "МЕНЮ")
RES_TEXT(SCR_MENU_TEMPERATURE, "Термометр")
RES_TEXT(SCR_MENU_TIME, "Час")
RES_TEXT(SCR_MENU_SETTINGS, "Налаштування")
RES_TEXT(SCR_MENU_ABOUT, "Інформація")
RES_TEXT(SCR_MENU_MISC, "Інше")

RES_TEXT(SCR_TIME_TITLE, "ЧАС")

RES_TEXT(SCR_SET_TIME_TITLE, "НАЛАШТ. ЧАСУ")

RES_TEXT(SCR_CONSTRUCTION_MSG_LINE_0, "У процесі")
RES_TEXT(SCR_CONSTRUCTION_MSG_LINE_1, "створення")

#endif //THERMOMETER_TEXT_RES_H
