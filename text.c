#include "text.h"
#include "utf8.h"
#include "utils.h"
#include "config.h"

#ifndef TEXT_SPACE_WIDTH
#error "TEXT_SPACE_WIDTH wasn't defined! Please, define it."
#endif

static FORCE_INLINE uint32_t __text_char_width(uint32_t utf8_char, GlyphLookupFunc_t glyph_lu_func) {
    if (utf8_char == ' ') {
        return TEXT_SPACE_WIDTH;
    }

    const uint8_t *glyph_data = glyph_lu_func(utf8_char);
    return glyph_data[0x0u];
}

uint32_t text_width(const uint8_t *str, GlyphLookupFunc_t glyph_lu_func, uint32_t char_count) {
    uint32_t width = 0u;
    uint32_t byte_index = 0u;
    uint32_t char_index = 0u;

    while (str[byte_index] != TEXT_NULL_CHAR && char_index < char_count) {
        uint32_t char_size = 0u;
        uint32_t utf8_char = utf8_get_char_code(str, char_index, &char_size);
        width += __text_char_width(utf8_char, glyph_lu_func);
        byte_index += char_size;
        char_index++;
    }

    return width;
}

uint32_t text_offset(uint32_t text_width, TextAlign_t align, uint32_t displace) {
    if (text_width >= DISPLAY_WIDTH || displace >= DISPLAY_WIDTH) {
        return 0u;
    }

    switch (align) {
        case TEXT_ALIGN_CENTER: {
            uint8_t max_width = DISPLAY_WIDTH - displace;
            if (max_width <= text_width) {
                return 0u;
            }

            return clamp_uint32_t((max_width - text_width) / 2u + displace, 0u, DISPLAY_WIDTH - 1u);
        }

        case TEXT_ALIGN_RIGHT: {
            uint8_t max_width = DISPLAY_WIDTH - displace;
            if (max_width <= text_width) {
                return 0u;
            }

            return clamp_uint32_t(max_width - text_width + displace, 0u, DISPLAY_WIDTH - 1u);
        }

        default: {
            return displace;
        }
    }
}
