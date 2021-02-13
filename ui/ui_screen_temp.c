#include "ui_screen_temp.h"
#include "../config.h"
#include "fonts/bebas_neue_16.h"
#include "fonts/arial_black_24.h"
#include "../text_res.h"
#include <stddef.h>
#include "../temp_sensor_dispatcher.h"

#define TEMP_BUFFER_LEN 16u

static uint32_t num_to_str(int32_t number, uint8_t *dest, uint32_t max_len) {
    if (dest == NULL || max_len < 2) {
        return 0;
    }

    if (number == 0) {
        dest[0] = '0';
        dest[1] = '\0';
        return 2;
    }

    uint32_t char_index = 0;
    if (number < 0) {
        number = -number;
        dest[char_index++] = '-';
    }

    bool trailing = true;

    for(uint32_t div = 1000000000; div > 0; div /= 10) {
        uint32_t digit = number / div;
        number %= div;

        if (trailing && digit == 0) {
            continue;
        }

        trailing = false;
        dest[char_index++] = (uint8_t) (digit + '0');

        if (char_index == (max_len - 1)) {
            break;
        }
    }

    dest[char_index++] = '\0';

    return char_index;
}

static uint8_t temp_str_buffer[TEMP_BUFFER_LEN];

void ui_screen_temp_draw(const UiDisplay_t *display) {
    display -> clear();

    uint8_t title_line_y = (SSD1306_PIXEL_PER_PAGE << 1u) + 2u;
    for (uint8_t x = 0; x < DISPLAY_WIDTH; x++) {
        display -> put_pixel(x, title_line_y);
    }

    display -> text_set_offset_x(0);
    display -> text_set_align(TEXT_ALIGN_CENTER);

    display -> text_set_page(0);
    display -> text_set_font(ssd1306_bebas_neue_16_get_glyph);
    display -> put_text(RES_STR_SCR_TEMP_TITLE);

    display -> text_set_page(3);
    display -> text_set_font(ssd1306_arial_black_24_get_glyph);

    if (num_to_str(tsd_get_t(), temp_str_buffer, TEMP_BUFFER_LEN)) {
        display -> put_text(temp_str_buffer);
    }
}
