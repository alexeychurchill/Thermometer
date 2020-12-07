#include "display.h"

#include "config.h"
#include "stm32f1xx.h"
#include "i2c.h"
#include "utf8.h"

#define SPACE_CHAR (' ')

static uint8_t display_buffer[DISPLAY_BUFFER_SIZE];

/**
 * TODO:
 * Make I2C-independent - verything should be sent through send_display function.
 * Rename to display_tx(const uint8_t*, uint32_t)
 */
static void send_display(const uint8_t* data, uint32_t length) {
    i2c_start(DISPLAY_I2C, DISPLAY_ADDR, I2C_MODE_TX);
    i2c_send(DISPLAY_I2C, data, length);
    i2c_stop(DISPLAY_I2C);
}

void display_init() {
    // Init I2C 2 for SSD1306-display
    i2c_setup_timing_sm(DISPLAY_I2C, SystemCoreClock / 2);
    i2c_enable(DISPLAY_I2C);

    // Init SSD1306
    ssd1306_set_display_off(send_display);
    ssd1306_set_multiplex_ratio(send_display, (DISPLAY_HEIGHT - 0x1));
    ssd1306_set_display_offset(send_display, 0x00);
    ssd1306_set_display_start_line(send_display, 0x00);
    ssd1306_segment_remap_off(send_display);
    ssd1306_set_com_scan_direction_remap(send_display);
    ssd1306_set_addr_mode(send_display, SSD1306_ADDR_MODE_VERTICAL);
    ssd1306_set_com_pins_config(send_display, DISPLAY_COM_PINS_CONFIG);
    // TODO: Make contrast adjustable from the device menu!
    ssd1306_set_contrast(send_display, DISPLAY_CONTRAST);
    ssd1306_entire_display_off(send_display);
    ssd1306_set_inverse_off(send_display);
    ssd1306_set_oscillator_freq_and_divider(send_display, 0x8, 0x0);
    ssd1306_setup_charge_pump(send_display, SSD1306_CHARGE_PUMP_ENABLE);
    ssd1306_set_display_on(send_display);
}

uint8_t* display_get_buffer() {
    return display_buffer;
}

void display_buffer_clear() {
    for (uint32_t page_index = 0x0; page_index < DISPLAY_BUFFER_SIZE; page_index++) {
        display_buffer[page_index] = 0x0;
    }
}

void display_buffer_put_pixel(uint8_t x, uint8_t y) {
    uint32_t buf_offset = (y / SSD1306_PIXEL_PER_PAGE) * DISPLAY_WIDTH + x;
    uint8_t page = display_buffer[buf_offset];
    page |= ((uint8_t) 0x1u) << (y % SSD1306_PIXEL_PER_PAGE);
    display_buffer[buf_offset] = page;
}

/**
 * TODO: Try to refactor cycle into macro which allows to define functions
 * for iteration over UTF-8 strings
 *
 * @param text
 * @param lookup_func
 * @param max_length
 * @param page
 * @param offset_x
 * @param align
 */
void display_buffer_put_text(
        const uint8_t text[],
        const uint8_t* (*lookup_func)(uint32_t utf8_code),
        uint32_t max_length,
        uint8_t page,
        uint8_t offset_x,
        TextAlign_t align
) {
    uint32_t char_index = 0;
    uint8_t hor_offset = 0;

    while (text[char_index] != '\0' && char_index < max_length) {
        if (text[char_index] == SPACE_CHAR) {
            char_index++;
            hor_offset += DISPLAY_SPACE_WIDTH;
            continue;
        }

        uint32_t char_byte_len;
        uint32_t char_utf8 = utf8_get_char_code(text, char_index, &char_byte_len);
        char_index += char_byte_len;

        const uint8_t *glyph = lookup_func(char_utf8);
        uint8_t glyph_w = glyph[0x0], glyph_h = glyph[0x1];
        uint8_t glyph_pages = glyph_h / SSD1306_PIXEL_PER_PAGE;
        uint32_t glyph_byte_len = glyph_w * glyph_pages;

        for (uint32_t byte_index = 0x0; byte_index < glyph_byte_len; byte_index++) {
            uint32_t buf_page_offset = (byte_index / glyph_w) * DISPLAY_WIDTH;
            uint32_t buf_index = hor_offset + buf_page_offset + (byte_index % glyph_w);
            display_buffer[buf_index] |= glyph[0x02 + byte_index];
        }

        hor_offset += glyph_w;
    }
}

void display_flush() {
    ssd1306_set_addr_mode(send_display, SSD1306_ADDR_MODE_HORIZONTAL);
    ssd1306_vh_addr_set_page(send_display, 0x0, 0x7);
    ssd1306_vh_addr_set_column(send_display, 0x0, 0x7F);

    i2c_start(DISPLAY_I2C, DISPLAY_ADDR, I2C_MODE_TX);
    i2c_send_byte(DISPLAY_I2C, SSD1306_CTRL_DATA);

    for (uint32_t buf_index = 0x0; buf_index < sizeof(display_buffer); buf_index++) {
        i2c_send_byte(DISPLAY_I2C, display_buffer[buf_index]);
    }

    i2c_stop(DISPLAY_I2C);
}
