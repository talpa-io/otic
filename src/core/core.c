//
// Created by hp on 9/18/19.
//

#include "core.h"

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

uint8_t leb128_encode_unsigned(uint32_t value, uint8_t* restrict dest)
{
    static uint8_t byte;
    static uint8_t counter;
    counter = 0;
    do {
        byte = value;
        value >>= 7u;
        byte &= ~0x80u;
        if (value != 0)
            byte |= 0x80u;
        dest[counter++] = byte;
    } while (value != 0);
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
    uint8_t size = sizeof(value) * 8;
    uint8_t byte = 0;
    uint8_t counter = 0;
    while (more)
    {
        byte = value;
        value >>= 7u;
        byte &= ~0x80u;
        if (negative)
            value |= (~0 << (size - 7));
        if (((value == 0) && !(byte >> 6u)) || ((value == -1) && (byte >> 6u)))
            more = 0;
        else
            byte |= 1u << 7;
        dest[counter++] = byte;
    }
    return counter;
}

uint8_t leb128_decode_signed(const uint8_t* restrict encoded_values, int64_t* restrict result)
{
    *result = 0;
    uint8_t shift = 0;
    uint8_t sizse = sizeof(uint64_t) * 8;
    uint8_t byte = 0;
    uint8_t counter = 0;
    do {
        byte = encoded_values[counter++];
        *result |= (byte & ~0x80u) << shift;
        shift += 7;
    } while (byte >> 7u);
    byte &= ~0x80u;
    if ((shift < sizse) && (byte >> 6u))
        *result |= (~0u << shift);
    return shift / 7;
}