#include "scr_time.h"
#include "ui_screen.h"
#include "scr_common.h"
#include "ui_mode_dispatcher.h"
#include "bebas_24.h"
#include "../text_res.h"
#include "../rtc.h"
#include "../time.h"

#define SCR_TIME_PAGE 3u

static void __scr_start();
static void __scr_draw(const UiDisplay_t *display);
static void __scr_handle_button(const HmiBtnEvent_t event);

const UiScreen_t SCR_TIME = {
        .start = __scr_start,
        .draw = __scr_draw,
        .handle_button = __scr_handle_button
};

static void __scr_start() {
    // No op
}

static void __scr_draw(const UiDisplay_t *display) {
    display->clear();

    scr_common_draw_title(display, RES_STR_SCR_TIME_TITLE);

    uint32_t time = rtc_get_time();
    uint8_t time_str[TIME_STR_LENGTH];
    time_to_str(time, time_str);

    display->text_set_page(SCR_TIME_PAGE);
    display->text_set_align(TEXT_ALIGN_CENTER);
    display->text_set_offset_x(0);
    display->text_set_font(ssd1306_bebas_24_get_glyph);

    display->put_text(time_str);
}

static void __scr_handle_button(const HmiBtnEvent_t event) {
    if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_LONG_PRESS) {
        ui_mode_dispr_set(UI_MODE_MENU);
    }
}
