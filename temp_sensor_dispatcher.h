#ifndef THERMOMETER_TEMP_SENSOR_DISPATCHER_H
#define THERMOMETER_TEMP_SENSOR_DISPATCHER_H

#include <stdint.h>
#include "onewire.h"

void tsd_init(const OwBusLine_t *line);

void tsd_dispatch_state();

int32_t tsd_get_t();

#endif //THERMOMETER_TEMP_SENSOR_DISPATCHER_H
