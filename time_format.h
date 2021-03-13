#ifndef THERMOMETER_TIME_FORMAT_H
#define THERMOMETER_TIME_FORMAT_H
#include <stdint.h>

#define TIME_STR_LENGTH 9u // 00:00:00[EOL]

void time_to_str(uint32_t time, uint8_t *out_str);

#endif //THERMOMETER_TIME_FORMAT_H
