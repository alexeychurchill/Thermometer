#ifndef THERMOMETER_MODE_H
#define THERMOMETER_MODE_H

typedef enum UiMode {
    UI_MODE_NONE = 0xFFFFFFFFu,

    UI_MODE_MENU = 0x0u,
    UI_MODE_TEMPERATURE,
    UI_MODE_TIME,
    UI_MODE_SETTINGS,
    UI_MODE_INFO,
    UI_MODE_SET_TIME,

    UI_MODE_COUNT,
} UiMode_t;

#endif //THERMOMETER_MODE_H
