#include "time_format.h"
#include "utils.h"

#define TIME_SEC_IN_DAY 86400u // 1 day * 24 hrs * 60 min * 60 sec
#define TIME_SEC_IN_MIN 60u // 60 sec
#define TIME_MIN_IN_HR 60u // 60 min
#define TIME_DIGIT_DIV 10u

#define TIME_START_CHAR ('0')
#define TIME_DELIMITER (':')
#define TIME_DIGIT_DIV 10u

static FORCE_INLINE void __time_add_delimiter(uint8_t *out_str, uint32_t *index) {
    out_str[*index] = TIME_DELIMITER;
    (*index)++;
}

static FORCE_INLINE void __time_add_number(uint8_t *out_str, uint32_t *index, uint32_t num) {
    out_str[*index] = (num / TIME_DIGIT_DIV) % TIME_DIGIT_DIV + TIME_START_CHAR;
    (*index)++;
    out_str[*index] = num % TIME_DIGIT_DIV + TIME_START_CHAR;
    (*index)++;
}

static FORCE_INLINE void __time_add_eol(uint8_t *out_str, uint32_t *index) {
    out_str[*index] = '\0';
    (*index)++;
}

void time_to_str(uint32_t time, uint8_t *out_str) {
    uint32_t day_time = time % TIME_SEC_IN_DAY;

    uint32_t sec = day_time % TIME_SEC_IN_MIN;
    day_time /= TIME_SEC_IN_MIN;

    uint32_t min = day_time % TIME_MIN_IN_HR;
    // After this line will contain HRS
    day_time /= TIME_MIN_IN_HR;

    uint32_t index = 0;
    __time_add_number(out_str, &index, day_time);
    __time_add_delimiter(out_str, &index);
    __time_add_number(out_str, &index, min);
    __time_add_delimiter(out_str, &index);
    __time_add_number(out_str, &index, sec);
    __time_add_eol(out_str, &index);
}
