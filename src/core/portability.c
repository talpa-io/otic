#include "core/portability.h"

#if !OTIC_HAS_ENDIAN_H

__attribute__((always_inline)) uint16_t otic_bswap16(uint16_t value)
{
    return ((value & 0xFF00) >> 8u) | ((value & 0x00FF) << 8u);
}

__attribute__((always_inline)) uint32_t otic_bswap32(uint32_t value)
{
    return ((value & 0xFF000000) >> 24u) | ((value & 0x00FF0000) >> 8u) | ((value & 0x0000FF00) << 8u) | ((value & 0x000000FF) << 24u);
}

__attribute__((always_inline)) uint64_t otic_bswap64(uint64_t value)
{
    return ((value & 0xFF00000000000000ull) >> 56u) | ((value & 0x00FF000000000000ull) >> 40u) | ((value & 0x0000FF0000000000ull) >> 24u) | ((value & 0x000000FF00000000ull) >> 8u) | ((value & 0x00000000FF000000ull) << 8u) | ((value & 0x0000000000FF0000ull) << 24u) | ((value & 0x000000000000FF00ull) << 40u) | ((value & 0x00000000000000FFull) << 56u);
}

#endif // OTIC_HAS_ENDIAN_H
