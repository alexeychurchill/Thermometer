#include "ui_scr_common.h"
#include "../ssd1306.h"
#include "../display.h"

void ui_scr_draw_title(const UiDisplay_t *display, const uint8_t *title_text) {
    uint8_t title_line_y = UI_SCR_COMMON_TITLE_DELIMITER_Y;
    for (uint8_t x = 0; x < DISPLAY_WIDTH; x++) {
        display -> put_pixel(x, title_line_y);
    }

    display -> text_set_offset_x(0);
    display -> text_set_page(0);
    display -> text_set_align(TEXT_ALIGN_CENTER);

    display -> put_text(title_text);
}
