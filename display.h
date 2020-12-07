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
void display_buffer_put_pixel(uint8_t x, uint8_t y);
void display_buffer_put_text(
        const uint8_t text[],
        const uint8_t* (*lookup_func)(uint32_t utf8_code),
        uint32_t max_length,
        uint8_t page,
        uint8_t offset_x,
        TextAlign_t align
);

void display_flush();

#endif //THERMOMETER_DISPLAY_H
