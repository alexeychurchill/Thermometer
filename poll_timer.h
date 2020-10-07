#ifndef THERMOMETER_POLL_TIMER_H
#define THERMOMETER_POLL_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx.h"

void poll_timer_init(TIM_TypeDef *tim);

void poll_timer_start(uint16_t duration);

bool poll_timer_is_running();

#endif //THERMOMETER_POLL_TIMER_H
