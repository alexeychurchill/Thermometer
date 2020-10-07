#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define FORCE_INLINE                        inline __attribute__((always_inline))

#define UINT8_BIT_COUNT                     8
#define UINT8_BIT_MASK(bit_n)               (((uint8_t)0x1) << (bit_n))
#define UINT8_BIT_VALUE(number, bit_n)      (((number) & UINT8_BIT_MASK(bit_n)) >> (bit_n))

#define DEF_ABS(intype, outtype)            FORCE_INLINE outtype abs_##intype(const intype x) { \
                                                return (outtype) (x >= 0 ? x : -x);              \
                                            }

DEF_ABS(int32_t, uint32_t)
DEF_ABS(int16_t, uint16_t)
DEF_ABS(int8_t, uint8_t)

#endif
