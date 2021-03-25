#ifndef THERMOMETER_BUTTONS_H
#define THERMOMETER_BUTTONS_H

#include <stddef.h>
#include <stdbool.h>


typedef enum HmiBtn {
    HMI_BTN_NONE,
    HMI_BTN_LEFT,
    HMI_BTN_ENTER,
    HMI_BTN_RIGHT
} HmiBtn_t;

typedef enum HmiBtnEventType {
    HMI_BTN_EVENT_NONE,
    HMI_BTN_EVENT_PRESS,
    HMI_BTN_EVENT_LONG_PRESS
} HmiBtnEventType_t;

typedef struct HmiBtnEvent {
    volatile HmiBtn_t btn;
    volatile HmiBtnEventType_t type;
} HmiBtnEvent_t;

void hmi_btn_init();

const bool hmi_btn_has_event();

const HmiBtnEvent_t hmi_btn_poll_event();

#endif //THERMOMETER_BUTTONS_H
