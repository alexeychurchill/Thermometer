#ifndef THERMOMETER_POWER_H
#define THERMOMETER_POWER_H

#include <stdbool.h>

typedef enum PwrState {
    PWR_STATE_RUNNING,
    PWR_STATE_BG_WORK,
    PWR_STATE_SLEEPING,
} PwrState_t;

typedef enum PwrWkupSource {
    PWR_WKUP_SOURCE_NONE = 0u,
    PWR_WKUP_SOURCE_BUTTON,
    PWR_WKUP_SOURCE_ALARM,
} PwrWkupSource_t;

void pwr_schedule_sleep();

void pwr_sleep_tick();

bool pwr_handle_wkup_isr(PwrWkupSource_t wkup_source);

void pwr_sleep();

bool pwr_poll_alarm();

bool pwr_poll_woke_up();

PwrState_t pwr_state();

void pwr_sleep_allow();

void pwr_sleep_disallow();

#endif //THERMOMETER_POWER_H
