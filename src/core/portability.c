#include "core/portability.h"

#if !OTIC_HAS_ENDIAN_H

inline uint16_t otic_bswap16(uint16_t value)
{
    return ((value & 0xFF00u) >> 8u) | ((value & 0x00FFu) << 8u);
}

inline uint32_t otic_bswap32(uint32_t value)
{
    return ((value & 0xFF000000u) >> 24u) | ((value & 0x00FF0000u) >> 8u) | ((value & 0x0000FF00u) << 8u)
    | ((value & 0x000000FFu) << 24u);
}

inline uint64_t otic_bswap64(uint64_t value)
{
    return ((value & 0xFF00000000000000ull) >> 56u) | ((value & 0x00FF000000000000ull) >> 40u)
    | ((value & 0x0000FF0000000000ull) >> 24u) | ((value & 0x000000FF00000000ull) >> 8u) |
    ((value & 0x00000000FF000000ull) << 8u) | ((value & 0x0000000000FF0000ull) << 24u) |
    ((value & 0x000000000000FF00ull) << 40u) | ((value & 0x00000000000000FFull) << 56u);
}

#endif // OTIC_HAS_ENDIAN_H
