#ifndef THERMOMETER_TIME_FORMAT_H
#define THERMOMETER_TIME_FORMAT_H
#include <stdint.h>

#define TIME_STR_DELIMITER                              (':')
#define TIME_STR_DELIMITER_COUNT                        2u
#define TIME_STR_LENGTH                                 9u // 00:00:00[EOL]
#define TIME_HR_PER_DAY                                 24u
#define TIME_MIN_PER_HR                                 60u
#define TIME_SEC_PER_MIN                                60u

void time_to_str(uint32_t time, uint8_t *out_str);

#endif //THERMOMETER_TIME_FORMAT_H
