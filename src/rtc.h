#ifndef THERMOMETER_RTC_H
#define THERMOMETER_RTC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void rtc_init();

void rtc_set_time(uint32_t time);

uint32_t rtc_get_time();

void rtc_set_alarm(uint32_t time);

void rtc_remove_alarm();

bool rtc_has_alarm();

void rtc_clear_alarm();

#endif //THERMOMETER_RTC_H
