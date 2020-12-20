#ifndef THERMOMETER_DISPLAY_H
#define THERMOMETER_DISPLAY_H

#include <stdint.h>
#include "config.h"
#include "ssd1306.h"

typedef enum TextAlign {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT,
} TextAlign_t;

void display_init();
uint8_t* display_get_buffer();
void display_buffer_clear();
void display_buffer_clear_area(uint8_t start_x, uint8_t start_y, uint8_t width, uint8_t height);
void display_buffer_put_pixel(uint8_t x, uint8_t y);
void display_text_set_page(uint8_t page);
void display_text_set_offset_x(int8_t offset_x);
void display_text_set_align(TextAlign_t align);
void display_text_set_font(const uint8_t* (*glyph_lookup_func)(uint32_t));
void display_buffer_put_text(const uint8_t text[], uint32_t max_length);

void display_flush();

#endif //THERMOMETER_DISPLAY_H
