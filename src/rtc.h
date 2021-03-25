#ifndef THERMOMETER_RTC_H
#define THERMOMETER_RTC_H

#include <stddef.h>
#include <stdint.h>

void rtc_init();

void rtc_set_time(uint32_t time);

const uint32_t rtc_get_time();

#endif //THERMOMETER_RTC_H
