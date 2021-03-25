#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define FORCE_INLINE                        inline __attribute__((always_inline))

#define UINT8_BIT_COUNT                     8
#define UINT8_BIT_MASK(bit_n)               (((uint8_t)0x1) << (bit_n))
#define UINT8_BIT_VALUE(number, bit_n)      (((number) & UINT8_BIT_MASK(bit_n)) >> (bit_n))

#define LENGTH_OF(array)                    ({  \
    uint32_t sz = 0;                            \
    if (sizeof(array) > 0) {                    \
        sz = sizeof(array) / sizeof(array[0]);  \
    }                                           \
    sz;                                         \
})

#define DEF_ABS(intype, outtype)            FORCE_INLINE outtype abs_##intype(const intype x) { \
                                                return (outtype) (x >= 0 ? x : -x);              \
                                            }

#define DEF_CLAMP(type)                     FORCE_INLINE type clamp_##type( \
                                                const type x,               \
                                                const type min,             \
                                                const type max              \
                                            ) { \
                                                return ((x < min) ? min : ((x > max) ? max : x));\
                                            }

DEF_ABS(int32_t, uint32_t)
DEF_ABS(int16_t, uint16_t)
DEF_ABS(int8_t, uint8_t)

DEF_CLAMP(uint32_t)
DEF_CLAMP(int32_t)
DEF_CLAMP(uint16_t)
DEF_CLAMP(int16_t)
DEF_CLAMP(uint8_t)
DEF_CLAMP(int8_t)

#endif
