#ifndef THERMOMETER_UI_SCREEN_COMMON_H
#define THERMOMETER_UI_SCREEN_COMMON_H

#include "ui_screen.h"

#define UI_SCR_COMMON_TITLE_DELIMITER_Y (SSD1306_PIXEL_PER_PAGE << 1u)

void ui_scr_draw_title(const UiDisplay_t *display, const uint8_t *title_text);

#endif //THERMOMETER_UI_SCREEN_COMMON_H
