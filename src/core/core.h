#ifndef OTIC_CORE_H
#define OTIC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define OTIC_BASE_CACHE_SIZE 1024
#if OTIC_BASE_CACHE_SIZE < (255 * 2)
#error OTIC Pack requires a buffer cache bigger than twice the size of permitted string value length (255)
#endif
#ifndef __STDC_IEC_559__
#error OTIC requires IEEE 754 support for handling float values
#endif
#define OTIC_ENTRY_STRING_SIZE 32
#define OTIC_PACK_CACHE_SIZE 256
#define PTR_M 31
#define OTIC_TS_MULTIPLICATOR 10000
#define OTIC_MAGIC_SIZE 4


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

typedef enum
{
    OTIC_FEATURE_COMPRESSION_ZSTD,
    OTIC_FEATURE_COMPRESSION_ZLIB,
    OTIC_FEATURE_COMPRESSION_GZIP,
} otic_features_e;

typedef enum
{
    OTIC_META_TYPE_CHANNEL_DEFINE,
    OTIC_META_TYPE_CHANNEL_TYPE,
    OTIC_META_TYPE_COMPRESSION_METHOD,
    OTIC_META_TYPE_END_OF_META = 0xFF
} otic_meta_type_e;

typedef struct
{
    uint8_t magic[OTIC_MAGIC_SIZE];
    uint8_t features;
    uint8_t version;
} __attribute__((packed)) otic_header_t;

typedef struct
{
    uint8_t metaType;
    uint8_t channelId;
    uint16_t opt_value;
} __attribute__((packed)) otic_meta_channel_t;

typedef struct
{
    uint8_t channelId;
    uint32_t dataLen;
    uint64_t startTimestamp;
} __attribute__((packed)) otic_payload_t;

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

typedef enum {
    OTIC_CHANNEL_TYPE_SENSOR,
    OTIC_CHANNEL_TYPE_BINARY
} channel_type_e;

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