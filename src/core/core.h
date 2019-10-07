#ifndef OTIC_CORE_H
#define OTIC_CORE_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stddef.h>

#define OTIC_BASE_CACHE_SIZE 1024
#if OTIC_BASE_CACHE_SIZE < (255 * 2)
#error OTIC Pack requires a buffer cache bigger than twice the size of permitted string value length (255)
#endif
#define OTIC_ENTRY_STRING_SIZE 32
#define OTIC_PACK_CACHE_SIZE 256
#define PTR_M 31
#define OTIC_TS_MULTIPLICATOR 10000


typedef enum
{
    OTIC_TYPE_NULL,
    OTIC_TYPE_EMPTY_STRING,
    OTIC_TYPE_INT32_NEG,
    OTIC_TYPE_INT32_POS,
    OTIC_TYPE_DOUBLE,
    OTIC_TYPE_MIN1_FLOAT,
    OTIC_TYPE_MIN2_FLOAT,
    OTIC_TYPE_MIN3_FLOAT,
    OTIC_TYPE_FLOAT,
    OTIC_TYPE_MED_DOUBLE,
    OTIC_TYPE_STRING,
    OTIC_TYPE_UNMODIFIED,
    OTIC_TYPE_RAWBUFFER,
    OTIC_TYPE_SET_TIMESTAMP,
    OTIC_TYPE_SHIFT_TIMESTAMP,
    OTIC_TYPE_FILE_VERSION,
    OTIC_TYPE_NAME_ASSIGN,
    OTIC_TYPE_EOF,
    OTIC_TYPE_METADATA
} otic_types_e;

typedef enum
{
    OTIC_ERROR_NONE,
    OTIC_ERROR_INVALID_POINTER,
    OTIC_ERROR_BUFFER_OVERFLOW,
    OTIC_ERROR_INVALID_TIMESTAMP,
    OTIC_ERROR_ENTRY_INSERTION_FAILURE,
    OTIC_ERROR_ZSTD,
    OTIC_ERROR_FLUSH_FAILED,
    OTIC_ERROR_EOF,
    OTIC_ERROR_INVALID_FILE,
    OTIC_ERROR_DATA_CORRUPTED,
    OTIC_ERROR_VERSION_UNSUPPORTED,
    OTIC_ERROR_ROW_COUNT_MISMATCH
} otic_errors_e;

typedef enum
{
    OTIC_STATE_OPENED,
    OTIC_STATE_ON_ERROR,
    OTIC_STATE_CLOSED
} otic_state_e;

typedef struct otic_base_t otic_base_t;
struct otic_base_t
{
    uint8_t cache[OTIC_BASE_CACHE_SIZE];
    uint8_t* top;
    uint64_t timestamp_start;
    uint64_t timestamp_current;
    otic_errors_e error;
    otic_state_e state;
    size_t rowCounter;
};

uint8_t         otic_base_init(otic_base_t* base);
void            otic_base_setError(otic_base_t *base, otic_errors_e error);
otic_errors_e   otic_base_getError(otic_base_t *base);
void            otic_base_setState(otic_base_t* base, otic_state_e state);
otic_state_e    otic_base_getState(otic_base_t* base);

// TODO: Improve granularity and test signed versions
uint8_t         leb128_encode_unsigned(uint64_t value, uint8_t* dest);
uint8_t         leb128_decode_unsigned(const uint8_t* encoded_values, uint32_t* value);
uint8_t         leb128_encode_signed(int64_t value, uint8_t* dest);
uint64_t        leb128_decode_signed(const uint8_t* encoded_values);

#ifdef __cpluscplus
};
#endif

#endif // OTIC_CORE_H