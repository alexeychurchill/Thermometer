#include "scr_set_time.h"
#include "scr_common.h"
#include "text_res.h"
#include "bebas_24.h"
#include "util/utf8.h"
#include "time.h"
#include "rtc.h"
#include "../mode/ui_mode_dispatcher.h"

static const uint32_t SET_TIME_DIGIT_COUNT = 6u;
static const uint8_t CURRENT_DIGIT_PAGE_START = 2u;
static const uint8_t CURRENT_DIGIT_PAGE_END = 6u;

static uint32_t s_current_digit = 0u;

static uint8_t s_time_str_buf[TIME_STR_LENGTH] = "00:00:00";

static void start();
static void draw(const UiDisplay_t *display);
static void handle_button(HmiBtnEvent_t event);

const UiScreen_t SCR_SET_TIME = {
        .start = start,
        .draw = draw,
        .handle_button = handle_button,
};

static void time_dec() {
    uint32_t now = rtc_get_time();

    now = time_digit_dec(now, s_current_digit);

    rtc_set_time(now);
}

static void time_inc() {
    uint32_t now = rtc_get_time();

    now = time_digit_inc(now, s_current_digit);

    rtc_set_time(now);
}

static void show_active_digit(const UiDisplay_t *display, uint8_t start_x, uint8_t end_x) {
    for (uint8_t page = CURRENT_DIGIT_PAGE_START; page <= CURRENT_DIGIT_PAGE_END; ++page) {
        display->invert(page, start_x, end_x);
    }
}

static void start() {
    s_current_digit = 0;
}

static void draw(const UiDisplay_t *display) {
    display->clear();

    scr_common_draw_title(display, RES_STR_SCR_SET_TIME_TITLE);

    display->text_set_offset_x(0u);
    display->text_set_page(3u);
    display->text_set_align(TEXT_ALIGN_CENTER);
    display->text_set_font(ssd1306_bebas_24_get_glyph);

    uint32_t current_time = rtc_get_time();

    time_to_str(current_time, s_time_str_buf);

    display->put_text(s_time_str_buf);

    uint32_t delimiter_count = s_current_digit / TIME_STR_DELIMITER_COUNT;
    uint32_t digit_index = s_current_digit + delimiter_count;

    uint32_t time_width = text_width(s_time_str_buf, ssd1306_bebas_24_get_glyph, TEXT_CHAR_COUNT_ALL);
    uint32_t display_offset = text_offset(time_width, TEXT_ALIGN_CENTER, 0u);

    uint32_t current_char = utf8_get_char_code(s_time_str_buf, digit_index, NULL);
    const uint8_t *glyph = ssd1306_bebas_24_get_glyph(current_char);
    uint32_t current_char_width = glyph[0x0u];

    uint32_t selection_offset = text_width(s_time_str_buf, ssd1306_bebas_24_get_glyph, digit_index);
    uint32_t start_x = display_offset + selection_offset;
    uint32_t end_x = start_x + current_char_width - 2u;

    show_active_digit(display, start_x, end_x);
}

static void handle_button(const HmiBtnEvent_t event) {
    if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_LONG_PRESS) {
        ui_mode_dispr_set(UI_MODE_SETTINGS);
    } else if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_PRESS) {
        s_current_digit = (s_current_digit + 1u) % SET_TIME_DIGIT_COUNT;
    } else if (event.btn == HMI_BTN_LEFT && event.type == HMI_BTN_EVENT_PRESS) {
        time_dec();
    } else if (event.btn == HMI_BTN_RIGHT && event.type == HMI_BTN_EVENT_PRESS) {
        time_inc();
    }
}
