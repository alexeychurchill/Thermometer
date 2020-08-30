#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define FORCE_INLINE                        inline __attribute__((always_inline))

#define UINT8_BIT_COUNT                     8
#define UINT8_BIT_MASK(bit_n)               (((uint8_t)0x1) << (bit_n))
#define UINT8_BIT_VALUE(number, bit_n)      (((number) & UINT8_BIT_MASK(bit_n)) >> (bit_n))

#endif
