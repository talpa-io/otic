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
#ifdef __builtin_bswap16
#define bswap16(x) __builtin_bswap16(x)
#else
#define bswap16(x) (((uint16_t)x & 0xFF00) >> 8u) | (((uint16_t)x & 0x00FF) << 8u)
#endif

#ifdef __builtin_bswap32
#define bswap32(x) __builtin_bswap32(x)
#else
#define bswap32(x) (((uint32_t)x & 0xFF000000) >> 24u) | (((uint32_t)x & 0x00FF0000) >> 8u) \
        | (((uint32_t)x & 0x0000FF00) << 8u) | (((uint32_t)x & 0x000000FF) << 24u)
#endif

#ifdef __builtin_bswap64
#define bswap64(x) __builtin_bswap64(x)
#else
#define bswap64(x) (((uint64_t)x & 0xFF00000000000000ull) >> 56u) | (((uint64_t)x & 0x00FF000000000000ull) >> 40u) | \
        (((uint64_t)x & 0x0000FF0000000000ull) >> 24u) | (((uint64_t)x & 0x000000FF00000000ull) >> 8u) | \
        (((uint64_t)x & 0x00000000FF000000ull) << 8u) | (((uint64_t)x & 0x0000000000FF0000ull) << 24u) | \
        (((uint64_t)x & 0x000000000000FF00ull) << 40u) | (((uint64_t)x & 0x00000000000000FFull) << 56u)
#endif

#endif // OTIC_HAS_ENDIAN_H

#if __GNUC__ >= 3
#define ALWAYS_INLINE           inline __attribute__((always_inline))
#define NOINLINE                __attribute__((noinline))
#define PURE                    __attribute__((pure))
#define CONST                   __attribute__((const))
#define NORETURN                __attribute__((noreturn))
#define MALLOC                  __attribute__((malloc))
#define MUST_CHECK              __attribute__((warn_unused_result))
#define DEPRECATED              __attribute__((deprecated))
#define USED                    __attribute__((used))
#define UNUSED                  __attribute__((unused))
#define PACKED                  __attribute__((packed))
#define ALIGN(x)                __attribute__((aligned (x)))
#define ALIGN_MAX               __attribute__((aligned))
#define LIKELY(x)               __builtin_expect(!!(x), 1)
#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#else
#define NOINLINE
#define PURE
#define CONST
#define NORETURN
#define MALLOC
#define MUST_CHECK
#define DEPRECATED
#define USED
#define UNUSED
#define PACKED
#define ALIGN(x)
#define ALIGN_MAX
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

// TODO: Window __attribute__((packed)) equivalent #pragma push()

#ifdef OTIC_WIN32
#define OTIC_API __declspec(dllexport)
#define NONNULL(...)
#define MAXPACK
#elif defined(__GNUC__) && __GNUC__ >= 4
#define OTIC_API                __attribute__((visibility("default")))
#define NONNULL(...)            __attribute__((nonnull(__VA_ARGS__)))
#define PACK_MAX                __attribute__((packed))
#else
#define OTIC_API
#define NONNULL(...)
#define MAXPACK
#endif

#if defined(__GNUC__)
#define OTIC_STATIC_INLINE static __inline
#elif defined(__cplusplus) || defined(__GNUC__) && __STDC_VERSION__ >= 199901L
#define OTIC_STATIC_INLINE static inline
#elif defined(OTIC_WIN32)
#define OTIC_STATIC_INLINE static __inline
#else
#define OTIC_STATIC_INLINE static
#endif

#ifdef __cplusplus
}
#endif

#endif // OTIC_PORTABILITY_H
