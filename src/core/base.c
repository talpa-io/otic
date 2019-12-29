#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "core/base.h"


void otic_base_init(otic_base_t* base)
{
    base->top = base->cache;
    base->timestamp_current = base->rowCounter = 0;
    base->error = OTIC_ERROR_NONE;
    base->state = OTIC_STATE_OPENED;
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

otic_types_e otic_oval_getType(const oval_t* oval)
{
    return oval->type;
}

void otic_oval_setd(oval_t* oval, uint32_t value, uint8_t neg)
{
    oval->lval = value;
    oval->type = neg ? OTIC_TYPE_INT_NEG: OTIC_TYPE_INT_POS;
}

void otic_oval_setdp(oval_t* oval, uint32_t value)
{
    oval->type = OTIC_TYPE_INT_POS;
    oval->lval = value;
}

void otic_oval_setdn(oval_t* oval, uint32_t value)
{
    oval->type = OTIC_TYPE_INT_NEG;
    oval->lval = value;
}

void otic_oval_setlf(oval_t* oval, double value)
{
    oval->type = OTIC_TYPE_DOUBLE;
    oval->dval = value;
}

void otic_oval_sets(oval_t* oval, const char* value, size_t size)
{
    oval->type = OTIC_TYPE_STRING;
    oval->sval.ptr = (char*)value;
    oval->sval.size = size;
}

void otic_oval_setn(oval_t* oval)
{
    oval->type = OTIC_TYPE_NULL;
}

uint8_t otic_oval_isNumeric(oval_t* oval)
{
    return oval->type == OTIC_TYPE_DOUBLE || oval->type == OTIC_TYPE_INT_POS || oval->type == OTIC_TYPE_INT_NEG;
}

uint8_t otic_oval_cmp(const oval_t* val1, const oval_t* val2)
{
    if (val1->type != val2->type)
        return 0;
    switch (val1->type)
    {
        case OTIC_TYPE_NULL:
            return 1;
        case OTIC_TYPE_INT_POS:
        case OTIC_TYPE_INT_NEG:
            return val1->lval == val2->lval;
        case OTIC_TYPE_DOUBLE:
            return val1->dval == val2->dval;
        case OTIC_TYPE_STRING:
            if (val1->sval.size != val2->sval.size)
                return 0;
            return strncmp(val1->sval.ptr, val2->sval.ptr, val1->sval.size) == 0;    
    }
    return 0;
}

void otic_oval_cpy(oval_t* dest, const oval_t* source)
{
    memcpy(dest, source, sizeof(typeof(*source)));
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
 * Factoring the \a if s results in the following algorithm, which results in 17 ASM lines ouput from x86-64 GCC
 * @attention Notice that \a restrict is used despite the fact that the ptr is aliased (dest and p). We can force the compiler to
 * optimize the algorithm as one of the ptr doesn't change the values it is pointing to.
 * @return The number of bytes written into \a dest
 */
uint8_t leb128_encode_unsigned(uint32_t value, uint8_t* restrict dest)
{
    typeof(dest) ptr = dest;
    while (value >= 128)
    {
        *ptr++ = 0x80u | (value & 0x7Fu);
        value >>= 7u;
    }
    *ptr++ = (uint8_t)value;
    return ptr - dest;
}

/**
* @brief: In analogy to the \a leb128_encode_unsigned(), the algorithm from Wikipedia was improved here to improve the
* overall Performance. The following function outputs 17 lines of ASM when compiled -O1 with GCC 9.2 for an X86-64 machine
*/
uint8_t leb128_decode_unsigned(const uint8_t* restrict encoded_values, uint32_t* restrict value)
{
    typeof(encoded_values) ptr = encoded_values;
    uint8_t shift = 0;
    *value = 0;
    do {
        *value |= ((*ptr & 0x7Fu) << shift);
        shift += 7;
    } while (*ptr++ & 0x80);
    return ptr - encoded_values;
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
        byte = value & ~0x80uL;
        signBit = ((uint8_t)value &~0xBFuL) >> 6u;
        value >>= 7uL;
        if (negative)
            value |= (~0uL << (size - 7uL));
        if ((value == 0 && !signBit) || (value == -1 && signBit))
            more = 0;
        else
            byte |= 0x80uL;
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
    int8_t counter = 0;
    do {
        byte = encoded_values[counter++];
        *result |= (byte & ~0x80L) << shift;
        shift += 7;
    } while ((byte >> 7));
    if ((shift < size) && (byte & ~0xBFL) >> 6)
        *result |= (~0L << shift);
    return counter;
}

otic_str_t* otic_setStr(const char* ptr)
{
    otic_str_t* oticStr = malloc(sizeof(otic_str_t));
    oticStr->size = ptr ? strlen(ptr) : 0;
    oticStr->ptr = (char*)ptr;
    return oticStr;
}

void otic_freeStr(otic_str_t* oticStr)
{
    free(oticStr);
}

void otic_updateStr(otic_str_t* oticStr, const char* ptr)
{
    oticStr->size = ptr ? strlen(ptr) : 0;
    oticStr->ptr = (char*)ptr;
}
