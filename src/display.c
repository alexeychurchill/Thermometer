#include "display.h"

#include "config.h"
#include "stm32f1xx.h"
#include "interfaces/i2c.h"
#include "util/utf8.h"
#include "util/utils.h"

#define SPACE_CHAR                      (' ')

#define DISPLAY_PAGE_COUNT              (DISPLAY_HEIGHT / SSD1306_PIXEL_PER_PAGE)
#define DISPLAY_PAGE_MIN                0x0u
#define DISPLAY_PAGE_MAX                (DISPLAY_PAGE_COUNT - 1u)
#define DISPLAY_PAGE_FULL_GLOW          0xFFu

/**
 * This string length limit doesn't have any practical
 * purpose but to prevent overflows, infinite loops,
 * wrong strings without '\0' etc., i.e. just for safety.
 */
#define DISPLAY_STRING_MAX_LEN          65u


static uint8_t display_buffer[DISPLAY_BUFFER_SIZE];


// Text parameters
static uint8_t text_page = 0;
static int8_t text_offset_x = 0;
static TextAlign_t text_align = TEXT_ALIGN_LEFT;
static const uint8_t* (*text_glyph_lookup)(uint32_t) = NULL;


static uint32_t display_text_get_hor_offset(uint32_t text_width, TextAlign_t align, int8_t offset_x) {
    if (text_width >= DISPLAY_WIDTH) {
        return 0u;
    }

    switch (align) {
        case TEXT_ALIGN_CENTER: {
            uint8_t max_width = DISPLAY_WIDTH - offset_x;
            if (max_width <= text_width) {
                return 0u;
            }

            return clamp_uint8_t((max_width - text_width) / 2 + offset_x, 0, DISPLAY_WIDTH - 1);
        }

        case TEXT_ALIGN_RIGHT: {
            uint8_t max_width = DISPLAY_WIDTH - offset_x;
            if (max_width <= text_width) {
                return 0u;
            }

            return clamp_uint8_t(max_width - text_width + offset_x, 0, DISPLAY_WIDTH - 1);
        }

        default: {
            return offset_x;
        }
    }
}

static uint32_t display_text_width(const uint8_t text[],const uint8_t* (*lookup_func)(uint32_t)) {
    uint32_t width = 0;
    uint32_t char_index = 0;
    while (text[char_index] != '\0' && char_index < DISPLAY_STRING_MAX_LEN) {
        if (text[char_index] == SPACE_CHAR) {
            char_index++;
            width += DISPLAY_SPACE_WIDTH;
            continue;
        }

        uint32_t char_byte_len;
        uint32_t char_utf8 = utf8_get_char_code(text, char_index, &char_byte_len);
        char_index += char_byte_len;

        const uint8_t *glyph = lookup_func(char_utf8);
        uint8_t glyph_w = glyph[0x0];

        width += glyph_w;
    }

    return width;
}

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

void display_off() {
    ssd1306_set_display_off(send_display);
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

void display_text_set_page(uint8_t page) {
    text_page = clamp_uint8_t(page, DISPLAY_PAGE_MIN, DISPLAY_PAGE_MAX);
}

void display_text_set_offset_x(int8_t offset_x) {
    text_offset_x = offset_x;
}

void display_text_set_align(TextAlign_t align) {
    text_align = align;
}

void display_text_set_font(const uint8_t* (*glyph_lookup_func)(uint32_t)) {
    text_glyph_lookup = glyph_lookup_func;
}

/**
 * TODO: Try to refactor cycle into macro which allows to define functions
 * for iteration over UTF-8 strings
 *
 * @param text
 */
void display_buffer_put_text(const uint8_t text[]) {
    if (text_glyph_lookup == NULL) {
        return;
    }

    uint32_t text_w = display_text_width(text, text_glyph_lookup);
    uint8_t hor_offset = display_text_get_hor_offset(text_w, text_align, text_offset_x);

    uint32_t char_index = 0;

    while (text[char_index] != '\0' && char_index < DISPLAY_STRING_MAX_LEN) {
        if (text[char_index] == SPACE_CHAR) {
            char_index++;
            hor_offset += DISPLAY_SPACE_WIDTH;
            continue;
        }

        uint32_t char_byte_len;
        uint32_t char_utf8 = utf8_get_char_code(text, char_index, &char_byte_len);
        char_index += char_byte_len;

        const uint8_t *glyph = text_glyph_lookup(char_utf8);
        uint8_t glyph_w = glyph[0x0], glyph_h = glyph[0x1];
        uint8_t glyph_pages = glyph_h / SSD1306_PIXEL_PER_PAGE;
        uint32_t glyph_byte_len = glyph_w * glyph_pages;

        if ((hor_offset + glyph_w) > DISPLAY_WIDTH) {
            break;
        }

        for (uint32_t byte_index = 0x0; byte_index < glyph_byte_len; byte_index++) {
            uint8_t buf_page = text_page + byte_index / glyph_w;
            uint32_t buf_page_offset = buf_page * DISPLAY_WIDTH;
            uint32_t buf_index = hor_offset + buf_page_offset + (byte_index % glyph_w);
            display_buffer[buf_index] |= glyph[0x02 + byte_index];
        }

        hor_offset += glyph_w;
    }
}

void display_buffer_invert(uint8_t page, uint8_t x_start, uint8_t x_end) {
    uint32_t buf_offset = page * DISPLAY_WIDTH;
    for (uint8_t x = x_start; x <= x_end; x++) {
        display_buffer[buf_offset + x] ^= DISPLAY_PAGE_FULL_GLOW;
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
