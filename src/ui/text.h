#ifndef THERMOMETER_TEXT_H
#define THERMOMETER_TEXT_H
#include <stdint.h>

#define TEXT_NULL_CHAR                                      '\0'
#define TEXT_CHAR_COUNT_ALL                                 0xFFFFFFFFu

/**
 * Glyph lookup function for the particular font.
 * Must return pointer to the beginning of the glyph data.<br>
 * First two bytes must be <b><i>width</i></b> and <b><i>height</i></b>
 * of the glyph.<br>
 * <br>
 * See <a href="https://github.com/alexeychurchill/SSD1306Fnt">SSD1306Fnt</a>
 * for example, to understand the principles.
 */
typedef const uint8_t* (GlyphLookupFunc_t)(uint32_t);

/** Text alignment related to the screen edges */
typedef enum TextAlign {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT,
} TextAlign_t;

/**
 * Returns width of the string (or, it's part) for specified font.<br>
 * <br>
 * <b>WARNING:</b> String MUST end with null-char! Otherwise, overflow is
 * guaranteed in case usage of TEXT_CHAR_COUNT_ALL.
 *
 * @param str ptr to the UTF-8 string (or, to the required char in the str)
 * @param glyph_lu_func lookup function for the required font
 * @param char_count count of the chars in the string which needed
 *        to be measured
 * @return width of string
 */
uint32_t text_width(const uint8_t *str, GlyphLookupFunc_t glyph_lu_func, uint32_t char_count);

/**
 * Calculates offset from the edge of the screen for text.<br>
 * @param text_width text width
 * @param align text edge aligning option
 * @param displace text displacement from the <b><i>picked align option</i></b>
 * @return offset of the text, from left edge
 */
uint32_t text_offset(uint32_t text_width, TextAlign_t align, uint32_t displace);

#endif //THERMOMETER_TEXT_H
