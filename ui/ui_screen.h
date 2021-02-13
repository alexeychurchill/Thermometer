#ifndef THERMOMETER_UI_SCREEN_H
#define THERMOMETER_UI_SCREEN_H

#include <stdint.h>
#include <stdbool.h>
#include "../text.h"
#include "../buttons.h"


typedef struct UiDisplay {
    void (*clear)();
    void (*put_pixel)(uint8_t x, uint8_t y);
    void (*text_set_page)(uint8_t page);
    void (*text_set_offset_x)(int8_t offset_x);
    void (*text_set_align)(TextAlign_t align);
    void (*text_set_font)(const uint8_t* (*glyph_lookup_func)(uint32_t));
    void (*put_text)(const uint8_t text[]);
    void (*invert)(uint8_t page, uint8_t x_start, uint8_t x_end);
} UiDisplay_t;

typedef struct UiScreen {
    void (*draw)(const UiDisplay_t*);
    void (*handle_button)(const HmiBtnEvent_t event);
} UiScreen_t;

#endif //THERMOMETER_UI_SCREEN_H
