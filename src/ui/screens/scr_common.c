#include "scr_common.h"
#include "drivers/ssd1306.h"
#include "display.h"
#include "bebas_16.h"

void scr_common_draw_title(const UiDisplay_t *display, const uint8_t *title_text) {
    uint8_t title_line_y = SCR_COMMON_TITLE_DELIMITER_Y;
    for (uint8_t x = 0; x < DISPLAY_WIDTH; x++) {
        display->put_pixel(x, title_line_y);
    }

    display->text_set_font(ssd1306_bebas_16_get_glyph);
    display->text_set_offset_x(0);
    display->text_set_page(0);
    display->text_set_align(TEXT_ALIGN_CENTER);

    display->put_text(title_text);
}
