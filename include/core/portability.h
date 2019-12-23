#ifndef OTIC_PORTABILITY_H
#define OTIC_PORTABILITY_H

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
#define bswap16(x) otic_bswap16(x)
#endif

#ifdef __builtin_bswap32
#define bswap32(x) __builtin_bswap32(x)
#else
#define bswap32(x) otic_bswap32(x)
#endif


#ifdef __builtin_bswap64
#define bswap64(x) __builtin_bswap64(x)
#else
#define bswap64(x) otic_bswap64(x)
#endif

#endif // OTIC_HAS_ENDIAN_H

#endif // OTIC_PORTABILITY_H
