//
// Created by hp on 9/18/19.
//

#include "core.h"
#include <limits.h>
#include <string.h>
#include <stdlib.h>

uint8_t otic_base_init(otic_base_t* base)
{
    if (!base)
        return 0;
    base->top = base->cache;
    base->timestamp_current = base->timestamp_current = base->rowCounter = 0;
    base->error = OTIC_ERROR_NONE;
    base->state = OTIC_STATE_OPENED;
    return 1;
}

inline void otic_base_setError(otic_base_t * base, otic_errors_e error)
{
    base->error = error;
}

inline otic_errors_e otic_base_getError(otic_base_t *base)
{
    return base->error;
}

inline void otic_base_setState(otic_base_t* base, otic_state_e state)
{
    base->state = state;
}

inline otic_state_e otic_base_getState(otic_base_t* base)
{
    return base->state;
}

inline void otic_base_close(otic_base_t* base)
{
    base->state = OTIC_STATE_CLOSED;
}

/**
 * Encode Integral values or more precisely uint32_t values to leb128.
 * @param value
 * @param dest
 * @attention The algorithm provided on Wikipedia was updated here for performance:
 * Notice:
 * Following the steps from Wikipedia, the following algorithm is resulted
 * \code{.c}
 *      do {
 *          byte = value;
 *          value >>= 7u;
 *          byte &= ~0x80u;
 *          if (value != 0)
 *              byte |= 0x80u;
 *          dest[counter++] = byte;
 *      } while (value != 0);
 * \endcode
 * Factoring the \a if s results in the following algorithm
 * @return The number of bytes written into \a dest
 */
uint8_t leb128_encode_unsigned(uint32_t value, uint8_t* restrict dest)
{
    static uint8_t byte;
    static uint8_t counter;
    counter = 0;
    do {
        byte = value;
        value >>= 7u;
        byte &= ~0x80u;
        dest[counter++] = byte | 0x80u;
    } while (value != 0);
    dest[counter - 1] &= ~0x80u;
    return counter;
}

uint8_t leb128_decode_unsigned(const uint8_t* restrict encoded_values, uint32_t* restrict value)
{
    static uint8_t shift;
    static uint8_t byte;
    static uint8_t counter;
    *value = 0;
    counter = 0;
    shift = 0;
    while (1) {
        byte = encoded_values[counter++];
        *value |= (byte &~0x80u) << shift;
        if (!(byte >> 7u))
            break;
        shift += 7;
    }
    return counter;
}

uint8_t leb128_encode_signed(int64_t value, uint8_t* restrict dest)
{
    uint8_t more = 1;
    uint8_t negative = (value < 0);
    uint8_t size = sizeof(value) * CHAR_BIT;
    uint8_t byte;
    uint8_t signBit;
    uint8_t counter = 0;
    while (more)
    {
        byte = value & ~0x80u;
        signBit = ((uint8_t)value &~0xBFu) >> 6u;
        value >>= 7u;
        if (negative)
            value |= (~0u << (size - 7u));
        if ((value == 0 && !signBit) || (value == -1 && signBit))
            more = 0;
        else
            byte |= 0x80u;
        dest[counter++] = byte;
    }
    return counter;
}

uint8_t leb128_decode_signed(const uint8_t* restrict encoded_values, int64_t* restrict result)
{
    *result = 0;
    uint8_t shift = 0;
    uint8_t size = sizeof(int64_t) * CHAR_BIT;
    uint8_t byte;
    uint8_t counter = 0;
    do {
        byte = encoded_values[counter++];
        *result |= (byte & ~0x80u) << shift;
        shift += 7;
    } while ((byte >> 7u));
    if ((shift < size) && (byte & ~0xBFu) >> 6u)
        *result |= (~0u << shift);
    return counter;
}

otic_str_t* otic_setStr(const char* ptr)
{
    otic_str_t* oticStr = malloc(sizeof(otic_str_t));
    if (!ptr) {
        oticStr->size = 0;
        oticStr->ptr = 0;
    } else {
        oticStr->ptr = ptr;
        oticStr->size = strlen(ptr);
    }
    return oticStr;
}

void otic_freeStr(otic_str_t* oticStr)
{
    free(oticStr);
}

void otic_updateStr(otic_str_t* oticStr, const char* ptr)
{
    if (!ptr){
        oticStr->ptr = 0;
        oticStr->size = 0;
        return;
    }
    oticStr->size = strlen(ptr);
    oticStr->ptr = ptr;
}
