#ifndef THERMOMETER_DISPLAY_H
#define THERMOMETER_DISPLAY_H

#include <stdint.h>
#include "config.h"
#include "drivers/ssd1306.h"
#include "text.h"

/**
 * Display control functions
 */

void display_init();
void display_off();

/**
 * Drawing functions
 */

uint8_t* display_get_buffer();

void display_buffer_clear();
void display_buffer_put_pixel(uint8_t x, uint8_t y);
void display_text_set_page(uint8_t page);
void display_text_set_offset_x(int8_t offset_x);
void display_text_set_align(TextAlign_t align);
void display_text_set_font(const uint8_t* (*glyph_lookup_func)(uint32_t));
void display_buffer_put_text(const uint8_t text[]);
void display_buffer_invert(uint8_t page, uint8_t x_start, uint8_t x_end);

void display_flush();

#endif //THERMOMETER_DISPLAY_H
