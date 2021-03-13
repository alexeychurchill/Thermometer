#include "scr_init.h"
#include "bebas_16.h"
#include "../text_res.h"

#define SCR_INIT_PAGE 3u

static void __scr_init_start();
static void __scr_init_draw(const UiDisplay_t *display);
static void __scr_init_handle_button(const HmiBtnEvent_t event);

const UiScreen_t SCR_INIT = {
        .start = __scr_init_start,
        .draw = __scr_init_draw,
        .handle_button = __scr_init_handle_button,
};

static void __scr_init_start() {
    // No op
}

static void __scr_init_draw(const UiDisplay_t *display) {
    display->clear();

    display->text_set_font(ssd1306_bebas_16_get_glyph);
    display->text_set_offset_x(0);
    display->text_set_align(TEXT_ALIGN_CENTER);
    display->text_set_page(SCR_INIT_PAGE);

    display->put_text(RES_STR_SCR_INIT_CAPTION);
}

static void __scr_init_handle_button(const HmiBtnEvent_t event) {
    // No op
    // This screen isn't intended to be shown in normal mode,
    // only while we're waiting for some peripherals to init
}
