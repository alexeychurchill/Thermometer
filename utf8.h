#ifndef THERMOMETER_UTF8_H
#define THERMOMETER_UTF8_H

#include <stdint.h>
#include <stddef.h>
#include "utils.h"

#define UTF8_ERROR                      0xFFFFFFFFu

// See https://en.wikipedia.org/wiki/UTF-8 for some algorithms
#define UTF8_1_BYTE_MAX_VALUE           0b01111111u

#define UTF8_2_BYTE_MSB_MASK            0b11100000u
#define UTF8_2_BYTE_MSB_VALUE           0b11000000u

#define UTF8_3_BYTE_MSB_MASK            0b11110000u
#define UTF8_3_BYTE_MSB_VALUE           0b11100000u

#define UTF8_4_BYTE_MSB_MASK            0b11111000u
#define UTF8_4_BYTE_MSB_VALUE           0b11110000u

#define UTF8_IS_1BYTE(byte0)            ((byte0) <= UTF8_1_BYTE_MAX_VALUE)
#define UTF8_IS_2BYTE(byte0)            (((byte0) & UTF8_2_BYTE_MSB_MASK) == UTF8_2_BYTE_MSB_VALUE)
#define UTF8_IS_3BYTE(byte0)            (((byte0) & UTF8_3_BYTE_MSB_MASK) == UTF8_3_BYTE_MSB_VALUE)
#define UTF8_IS_4BYTE(byte0)            (((byte0) & UTF8_4_BYTE_MSB_MASK) == UTF8_4_BYTE_MSB_VALUE)

#define UTF8_PART(offset, shl)          ((uint32_t) data[index + (offset)] << (shl))


FORCE_INLINE uint32_t utf8_char_size(const uint8_t byte0) {
    if (UTF8_IS_1BYTE(byte0)) {
        return 1u;
    }

    if (UTF8_IS_2BYTE(byte0)) {
        return 2u;
    }

    if (UTF8_IS_3BYTE(byte0)) {
        return 3u;
    }

    if (UTF8_IS_4BYTE(byte0)) {
        return 4u;
    }

    return UTF8_ERROR;
}

FORCE_INLINE uint32_t utf8_get_char_code(
        const uint8_t *data,
        const uint32_t index,
        uint32_t *char_size
) {
    uint32_t size = utf8_char_size(data[index]);
    if (char_size != NULL) {
        *char_size = size;
    }

    switch (size) {
        case 1u:
            return UTF8_PART(0, 0);
        case 2u:
            return UTF8_PART(0, 8) | UTF8_PART(1, 0);
        case 3u:
            return UTF8_PART(0, 16) | UTF8_PART(1, 8) | UTF8_PART(2, 0);
        case 4u:
            return UTF8_PART(0, 24) | UTF8_PART(1, 16) | UTF8_PART(2, 8) | UTF8_PART(3, 0);
        default:
            return UTF8_ERROR;
    }
}

#endif //THERMOMETER_UTF8_H
