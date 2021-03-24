#include "time.h"
#include "utils.h"

#define TIME_SEC_IN_DAY 86400u // 1 day * 24 hrs * 60 min * 60 sec
#define TIME_SEC_IN_MIN 60u // 60 sec
#define TIME_MIN_IN_HR 60u // 60 min
#define TIME_DIGIT_DIV 10u

#define TIME_START_CHAR ('0')
#define TIME_DELIMITER (':')
#define TIME_DIGIT_DIV 10u

static const uint32_t __TIME_DIGIT_COUNT = 6u;

static const uint32_t __TIME_DIGIT_INDEX_HRS_TENS = 0u;

static const uint32_t __TIME_DIGIT_INDEX_HRS_ONES = 1u;

static const uint32_t __TIME_DIGIT_MAX_VALUE_HRS_ONES_HRS_2 = 3u;

static const uint32_t __TIME_SEC_PER_DIGIT[] = {
        10u * TIME_MIN_PER_HR * TIME_SEC_PER_MIN,
        TIME_MIN_PER_HR * TIME_SEC_PER_MIN,
        10u * TIME_SEC_PER_MIN,
        TIME_SEC_PER_MIN,
        10u,
        1u
};

static const uint32_t __TIME_DIGIT_MAX_VALUE[] = {
        2u, 9u, 5u, 9u, 5u, 9u
};

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

uint32_t time_set_digit(uint32_t time, uint32_t index, uint32_t value) {
    if (index == 0u) {
        uint32_t lo_rem = time % __TIME_SEC_PER_DIGIT[index];
        time = value * __TIME_SEC_PER_DIGIT[index] + lo_rem;
    } else {
        uint32_t hi_div = __TIME_SEC_PER_DIGIT[index - 1u];
        uint32_t hi = time / hi_div;
        uint32_t hi_rem = time % hi_div;

        uint32_t lo_div = __TIME_SEC_PER_DIGIT[index];
        uint32_t lo_rem = hi_rem % lo_div;

        time = hi * hi_div + value * lo_div + lo_rem;
    }

    return time;
}

uint32_t time_get_digit(uint32_t time, uint32_t index) {
    if (index == 0u) {
        return time / __TIME_SEC_PER_DIGIT[index];
    } else {
        uint32_t hi_rem = time % __TIME_SEC_PER_DIGIT[index - 1u];
        return hi_rem / __TIME_SEC_PER_DIGIT[index];
    }
}

static uint32_t __time_digit_hrs_ones_clamp_to_3(uint32_t time) {
    uint32_t ones_digit = time_get_digit(time, __TIME_DIGIT_INDEX_HRS_ONES);
    if (ones_digit > __TIME_DIGIT_MAX_VALUE_HRS_ONES_HRS_2) {
        ones_digit = __TIME_DIGIT_MAX_VALUE_HRS_ONES_HRS_2;
    }
    return time_set_digit(time, __TIME_DIGIT_INDEX_HRS_ONES, ones_digit);
}

static uint32_t __time_digit_inc_hrs_tens(uint32_t time) {
    uint32_t digit = time_get_digit(time, __TIME_DIGIT_INDEX_HRS_TENS);

    digit = (digit + 1u) % (__TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_TENS] + 1u);
    if (digit >= __TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_TENS]) {
        time = __time_digit_hrs_ones_clamp_to_3(time);
    }

    return time_set_digit(time, __TIME_DIGIT_INDEX_HRS_TENS, digit);
}

static uint32_t __time_digit_inc_hrs_ones(uint32_t time) {
    uint32_t digit = time_get_digit(time, __TIME_DIGIT_INDEX_HRS_ONES);
    uint32_t tens_digit = time_get_digit(time, __TIME_DIGIT_INDEX_HRS_TENS);

    uint32_t digit_max = __TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_ONES];
    if (tens_digit >= __TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_TENS]) {
        digit_max = __TIME_DIGIT_MAX_VALUE_HRS_ONES_HRS_2;
    }

    digit = (digit + 1u) % (digit_max + 1u);
    return time_set_digit(time, __TIME_DIGIT_INDEX_HRS_ONES, digit);
}

uint32_t time_digit_inc(uint32_t time, uint32_t digit_index) {
    if (digit_index == __TIME_DIGIT_INDEX_HRS_TENS) {
        return __time_digit_inc_hrs_tens(time);
    }

    if (digit_index == __TIME_DIGIT_INDEX_HRS_ONES) {
        return __time_digit_inc_hrs_ones(time);
    }

    uint32_t digit = time_get_digit(time, digit_index);
    digit = (digit + 1u) % (__TIME_DIGIT_MAX_VALUE[digit_index] + 1u);
    return time_set_digit(time, digit_index, digit);
}

static uint32_t __time_digit_dec_hrs_tens(uint32_t time) {
    uint32_t digit = time_get_digit(time, __TIME_DIGIT_INDEX_HRS_TENS);

    if (digit == 0u) {
        digit = __TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_TENS];
    } else {
        digit--;
    }

    if (digit == __TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_TENS]) {
        time = __time_digit_hrs_ones_clamp_to_3(time);
    }

    return time_set_digit(time, __TIME_DIGIT_INDEX_HRS_TENS, digit);
}

static uint32_t __time_digit_dec_hrs_ones(uint32_t time) {
    uint32_t digit = time_get_digit(time, __TIME_DIGIT_INDEX_HRS_ONES);
    uint32_t digit_tens = time_get_digit(time, __TIME_DIGIT_INDEX_HRS_TENS);

    uint32_t digit_max = __TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_ONES];
    if (digit_tens == __TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_TENS]) {
        digit_max = __TIME_DIGIT_MAX_VALUE_HRS_ONES_HRS_2;
    }

    if (digit == 0u) {
        digit = digit_max;
    } else {
        digit--;
    }

    return time_set_digit(time, __TIME_DIGIT_INDEX_HRS_ONES, digit);
}

uint32_t time_digit_dec(uint32_t time, uint32_t digit_index) {
    if (digit_index == __TIME_DIGIT_INDEX_HRS_TENS) {
        return __time_digit_dec_hrs_tens(time);
    }

    if (digit_index == __TIME_DIGIT_INDEX_HRS_ONES) {
        return __time_digit_dec_hrs_ones(time);
    }

    uint32_t digit = time_get_digit(time, digit_index);
    uint32_t digit_max = __TIME_DIGIT_MAX_VALUE[digit_index];
    if (digit_index == __TIME_DIGIT_INDEX_HRS_ONES) {
        uint32_t digit_tens = time_get_digit(time, __TIME_DIGIT_INDEX_HRS_TENS);
        digit_max = (digit_tens == __TIME_DIGIT_MAX_VALUE[__TIME_DIGIT_INDEX_HRS_ONES])
                ? __TIME_DIGIT_MAX_VALUE_HRS_ONES_HRS_2
                : digit_max;
    }

    if (digit == 0u) {
        digit = digit_max;
    } else {
        digit--;
    }

    return time_set_digit(time, digit_index, digit);
}
