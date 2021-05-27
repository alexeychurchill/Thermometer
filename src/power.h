#ifndef THERMOMETER_POWER_H
#define THERMOMETER_POWER_H

#include <stdbool.h>

typedef enum PwrState {
    PWR_STATE_RUNNING,
    PWR_STATE_BG_WORK,
    PWR_STATE_SLEEPING,
} PwrState_t;

void pwr_schedule_sleep();

void pwr_sleep_tick();

void pwr_sleep();

bool pwr_poll_alarm();

PwrState_t pwr_state();

void pwr_sleep_allow();

void pwr_sleep_disallow();

#endif //THERMOMETER_POWER_H
