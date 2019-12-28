#ifndef OTIC_PORTABILITY_H
#define OTIC_PORTABILITY_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include "config.h"

#if OTIC_HAS_ENDIAN_H
#include <endian.h>
#else

uint16_t otic_bswap16(uint16_t value);
uint32_t otic_bswap32(uint32_t value);
uint64_t otic_bswap64(uint64_t value);

#ifdef __builtin_bswap16
#define bswap16(x) __builtin_bswap16(x)
#else 
#define bswap16(x) (((uint16_t)x & 0xFF00) >> 8u) | (((uint16_t)x & 0x00FF) << 8u)
#endif

#ifdef __builtin_bswap32
#define bswap32(x) __builtin_bswap32(x)
#else
#define bswap32(x) (((uint32_t)x & 0xFF000000) >> 24u) | (((uint32_t)x & 0x00FF0000) >> 8u) | (((uint32_t)x & 0x0000FF00) << 8u) | (((uint32_t)x & 0x000000FF) << 24u)
#endif

#ifdef __builtin_bswap64
#define bswap64(x) __builtin_bswap64(x)
#else
#define bswap64(x) (((uint64_t)x & 0xFF00000000000000ull) >> 56u) | (((uint64_t)x & 0x00FF000000000000ull) >> 40u) | (((uint64_t)x & 0x0000FF0000000000ull) >> 24u) | (((uint64_t)x & 0x000000FF00000000ull) >> 8u) | (((uint64_t)x & 0x00000000FF000000ull) << 8u) | (((uint64_t)x & 0x0000000000FF0000ull) << 24u) | (((uint64_t)x & 0x000000000000FF00ull) << 40u) | (((uint64_t)x & 0x00000000000000FFull) << 56u)
#endif

#endif // OTIC_HAS_ENDIAN_H

#ifdef __cplusplus
}
#endif

#endif // OTIC_PORTABILITY_H
