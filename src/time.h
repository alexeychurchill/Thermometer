#ifndef THERMOMETER_TIME_H
#define THERMOMETER_TIME_H
#include <stdint.h>

#define TIME_STR_DELIMITER                              (':')
#define TIME_STR_DELIMITER_COUNT                        2u
#define TIME_STR_LENGTH                                 9u // 00:00:00[EOL]
#define TIME_HR_PER_DAY                                 24u
#define TIME_MIN_PER_HR                                 60u
#define TIME_SEC_PER_MIN                                60u

void time_to_str(uint32_t time, uint8_t *out_str);

/**
 * Sets specified time of day digit in the given timestamp (seconds):<br>
 * <code>
 * HH:MM:SS - time<br>
 * 01 23 45 - digits indexes
 * </code><br>
 * where H - hours, M - minutes, S - seconds.<br>
 *
 * @param time input timestamp
 * @param index digit index
 * @param value desired value of digit
 * @return resulting timestamp
 */
uint32_t time_set_digit(uint32_t time, uint32_t index, uint32_t value);

/**
 * Returns digit from time of day
 *
 * @see time_set_digit
 * @param time timestamp
 * @param index index of digit
 * @return digit
 */
uint32_t time_get_digit(uint32_t time, uint32_t index);

/**
 * Increments digit in the time of day
 * @param time input timestamp
 * @param digit_index index of the digit
 * @return output timestamp
 */
uint32_t time_digit_inc(uint32_t time, uint32_t digit_index);

/**
 * Decrements digit in the time of day
 * @see time_digit_inc
 */
uint32_t time_digit_dec(uint32_t time, uint32_t digit_index);

#endif //THERMOMETER_TIME_H
