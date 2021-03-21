#include "scr_set_time.h"
#include "scr_common.h"
#include "../text_res.h"
#include "bebas_24.h"
#include "../utils.h"
#include "../utf8.h"
#include "../time_format.h"
#include "../text.h"
#include "../display.h"
#include "../uart.h"
#include "../rtc.h"

static const uint32_t __SET_TIME_DIGIT_COUNT = 6u;

static const uint32_t __SET_TIME_SEC_PER_DIGIT[] = {
        10u * TIME_MIN_PER_HR * TIME_SEC_PER_MIN,
        TIME_MIN_PER_HR * TIME_SEC_PER_MIN,
        10u * TIME_SEC_PER_MIN,
        TIME_SEC_PER_MIN,
        10u,
        1u
};

static const __SET_TIME_HRS_OVER20_LDIGIT = 2u;
static const __SET_TIME_HRS_OVER20_RDIGIT_MAX = 3u;

static const uint32_t __SET_TIME_DIGIT_MAX_VALUE[] = {
        2u, 9u, 5u, 9u, 5u, 9u
};

static uint32_t __set_time_current_digit = 0u;

static uint8_t __time_str_buf[TIME_STR_LENGTH] = "00:00:00";

static void __scr_set_time_start();
static void __scr_set_time_draw(const UiDisplay_t *display);
static void __scr_set_time_handle_button(const HmiBtnEvent_t event);

const UiScreen_t SCR_SET_TIME = {
        .start = __scr_set_time_start,
        .draw = __scr_set_time_draw,
        .handle_button = __scr_set_time_handle_button
};

static uint32_t __set_time_get_sel_digit(uint32_t digit) {
    uint32_t now = rtc_get_time();

    if (digit == 0u) {
        return now / __SET_TIME_SEC_PER_DIGIT[digit];
    } else {
        uint32_t hi_rem = now % __SET_TIME_SEC_PER_DIGIT[digit - 1u];
        return hi_rem / __SET_TIME_SEC_PER_DIGIT[digit];
    }
}

static void __set_time_set_sel_digit(uint32_t digit, uint32_t digit_value) {
    uint32_t now = rtc_get_time();

    if (digit == 0u) {
        uint32_t lo_rem = now % __SET_TIME_SEC_PER_DIGIT[digit];
        now = digit_value * __SET_TIME_SEC_PER_DIGIT[digit] + lo_rem;
    } else {
        uint32_t hi_div = __SET_TIME_SEC_PER_DIGIT[digit - 1u];
        uint32_t hi = now / hi_div;
        uint32_t hi_rem = now % hi_div;

        uint32_t lo_div = __SET_TIME_SEC_PER_DIGIT[digit];
        uint32_t lo_rem = hi_rem % lo_div;

        now = hi * hi_div + digit_value * lo_div + lo_rem;
    }

    rtc_set_time(now);
}

static FORCE_INLINE uint32_t __set_time_get_digit_max(uint32_t digit_index) {
    uint32_t digit_max = __SET_TIME_DIGIT_MAX_VALUE[digit_index];
    if (digit_index != 1u) {
        return digit_max;
    }

    uint32_t hr_l_digit = __set_time_get_sel_digit(0u);
    if (hr_l_digit == __SET_TIME_HRS_OVER20_LDIGIT) {
        digit_max = __SET_TIME_HRS_OVER20_RDIGIT_MAX;
    }

    return digit_max;
}

static void __scr_set_time_dec_delta() {
    uint32_t digit = __set_time_get_sel_digit(__set_time_current_digit);

    uint32_t digit_max = __set_time_get_digit_max(__set_time_current_digit);

    if (digit == 0u) {
        digit = digit_max;
    } else {
        digit--;
    }

    __set_time_set_sel_digit(__set_time_current_digit, digit);
}

static void __sct_set_time_inc_delta() {
    uint32_t digit = __set_time_get_sel_digit(__set_time_current_digit);

    uint32_t digit_max = __set_time_get_digit_max(__set_time_current_digit);

    if (__set_time_current_digit == 0u && digit >= 1u) {
        uint32_t hr_r_digit = __set_time_get_sel_digit(1u);
        if (hr_r_digit > 3u) {
            __set_time_set_sel_digit(1u, 3u);
        }
    }

    digit = (digit + 1u) % (digit_max + 1u);
    __set_time_set_sel_digit(__set_time_current_digit, digit);
}

static void __scr_set_time_start() {
    __set_time_current_digit = 0;
}

static void __scr_set_time_draw(const UiDisplay_t *display) {
    display->clear();

    scr_common_draw_title(display, RES_STR_SCR_SET_TIME_TITLE);

    display->text_set_offset_x(0u);
    display->text_set_page(3u);
    display->text_set_align(TEXT_ALIGN_CENTER);
    display->text_set_font(ssd1306_bebas_24_get_glyph);

    uint32_t current_time = rtc_get_time();

    time_to_str(current_time, __time_str_buf);

    display->put_text(__time_str_buf);

    uint32_t delimiter_count = __set_time_current_digit / TIME_STR_DELIMITER_COUNT;
    uint32_t digit_index = __set_time_current_digit + delimiter_count;

    uint32_t time_width = text_width(__time_str_buf, ssd1306_bebas_24_get_glyph, TEXT_CHAR_COUNT_ALL);
    uint32_t display_offset = display_text_get_hor_offset(time_width, TEXT_ALIGN_CENTER, 0u);

    uint32_t current_char = utf8_get_char_code(__time_str_buf, digit_index, NULL);
    const uint8_t *glyph = ssd1306_bebas_24_get_glyph(current_char);
    uint32_t current_char_width = glyph[0x0u];

    uint32_t selection_offset = text_width(__time_str_buf, ssd1306_bebas_24_get_glyph, digit_index);
    uint32_t start_x = display_offset + selection_offset;
    uint32_t end_x = start_x + current_char_width - 2u;

    display->invert(3u, start_x, end_x);
    display->invert(4u, start_x, end_x);
    display->invert(5u, start_x, end_x);
    display->invert(6u, start_x, end_x);
}

static void __scr_set_time_handle_button(const HmiBtnEvent_t event) {
    if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_PRESS) {
        __set_time_current_digit = (__set_time_current_digit + 1u) % __SET_TIME_DIGIT_COUNT;
    } else if (event.btn == HMI_BTN_LEFT && event.type == HMI_BTN_EVENT_PRESS) {
        __scr_set_time_dec_delta();
    } else if (event.btn == HMI_BTN_RIGHT && event.type == HMI_BTN_EVENT_PRESS) {
        __sct_set_time_inc_delta();
    }
}
