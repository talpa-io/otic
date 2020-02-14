#ifndef OTIC_BASE_H
#define OTIC_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "portability.h"

#define OTIC_BASE_CACHE_SIZE            12000
#if OTIC_BASE_CACHE_SIZE < (255 * 2)
#error OTIC Pack requires a buffer cache bigger than twice the size of permitted string value length (255)
#endif
#ifndef __STDC_IEC_559__
#error OTIC requires IEEE 754 support for handling float values
#endif
#define OTIC_ZSTD_COMPRESSION_LEVEL     7
#define OTIC_TS_MULTIPLICATOR           10000
#define OTIC_ENTRY_STRING_SIZE          32
#define OTIC_PACK_CACHE_SIZE            255
#define OTIC_MAGIC_SIZE                 4
#define PTR_M                           31
#define SMALL_INT_LIMIT                 0xC9
#define OTIC_MAGIC                      "O\xa9\x46\x35"
#define TS_NULL                         (uint64_t)-1


typedef enum
{
    OTIC_TYPE_NULL                      = SMALL_INT_LIMIT,
    OTIC_TYPE_INT_NEG,
    OTIC_TYPE_INT_POS,
    OTIC_TYPE_FLOAT,
    OTIC_TYPE_DOUBLE,
    OTIC_TYPE_STRING,
    OTIC_TYPE_ARRAY,
    OTIC_TYPE_OBJECT,
    OTIC_TYPE_TRUE,
    OTIC_TYPE_FALSE,
    OTIC_TYPE_RAWBUFFER,
    OTIC_TYPE_UNMODIFIED,
    OTIC_TYPE_SET_TIMESTAMP,
    OTIC_TYPE_SHIFT_TIMESTAMP,
    OTIC_TYPE_NAME_ASSIGN,
    OTIC_TYPE_EOF,
} otic_type_e;

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
    OTIC_ERROR_ROW_COUNT_MISMATCH,
    OTIC_ERROR_INVALID_ARGUMENT,
    OTIC_ERROR_AT_INVALID_STATE,
    OTIC_ERROR_ALLOCATION_FAILURE,
} otic_error_e;

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
} otic_feature_e;

typedef enum
{
    OTIC_META_TYPE_CHANNEL_DEFINE,
    OTIC_META_TYPE_CHANNEL_TYPE,
    OTIC_META_TYPE_COMPRESSION_METHOD,
    OTIC_META_TYPE_CHUNK_SIZE,
    OTIC_META_TYPE_DATA
} otic_meta_type_e;

typedef struct
{
    uint8_t magic[OTIC_MAGIC_SIZE];
    uint8_t features;                   // Reserved
    uint8_t version;
} PACK_MAX otic_header_t;

typedef struct
{
    uint8_t metaType;                   // otic_meta_type_e
    uint8_t channelId;
} PACK_MAX otic_meta_data_t;

typedef struct
{
    otic_meta_data_t meta;
    uint32_t size;
} PACK_MAX otic_meta_data_size_t;

typedef struct
{
    uint32_t dataLen;
    uint8_t channelId;
} PACK_MAX otic_payload_t;

typedef struct otic_base_t otic_base_t;
struct otic_base_t
{
    uint8_t* cache;
    uint8_t* top;
    uint64_t timestampStart;
    uint64_t timestampCurrent;
    otic_error_e error;
    otic_state_e state;
    size_t rowCounter;
    uint32_t cacheSize;
};

typedef struct time_interval_t
{
    double time_start;
    double time_end;
} time_interval_t;

typedef enum {
    OTIC_CHANNEL_TYPE_SENSOR,
    OTIC_CHANNEL_TYPE_BINARY
} channel_type_e;


uint8_t         otic_base_init(otic_base_t* base, uint32_t bucketSize) NONNULL(1);
void            otic_base_setError(otic_base_t *base, otic_error_e error) NONNULL(1);
otic_error_e    otic_base_getError(otic_base_t *base) NONNULL(1);
void            otic_base_setState(otic_base_t* base, otic_state_e state) NONNULL(1);
otic_state_e    otic_base_getState(otic_base_t* base) NONNULL(1);
void            otic_base_close(otic_base_t* base) NONNULL(1);


typedef struct
{
    char* ptr;
    size_t size;
} otic_str_t;

otic_str_t*     otic_setStr(const char* ptr);
void            otic_freeStr(otic_str_t* oticStr) NONNULL(1);
void            otic_updateStr(otic_str_t* oticStr, const char* ptr) NONNULL(1);

struct oval_t;
struct oval_obj_element_t;

typedef struct oval_t oval_t;

typedef struct
{
    uint32_t size;
    oval_t* elements;
} oval_array_t;

uint8_t         otic_array_init(oval_t* val);
uint8_t         otic_array_init_size(oval_t* val, size_t size);
uint8_t         otic_array_release(oval_t* val);

typedef struct
{
    uint32_t size;
    struct oval_obj_element_t* elements;
} oval_obj_t;

struct oval_t
{
    union {
        uint64_t        lval;
        double          dval;
        otic_str_t      sval;
        oval_array_t    aval;
        oval_obj_t      oval;
    } val;
    uint8_t type;                   // Active Type == OTIC_TYPE
};

typedef struct
{
    oval_t key;
    oval_t value;
} oval_obj_element_t;

void            otic_oval_setd(oval_t* oval, uint32_t value, uint8_t neg);
void            otic_oval_setdp(oval_t* oval, uint64_t value);
void            otic_oval_setdn(oval_t* oval, uint32_t value);
void            otic_oval_setlf(oval_t* oval, double value);
void            otic_oval_sets(oval_t* oval, const char* value, size_t size);
void            otic_oval_setn(oval_t* oval);
uint8_t         otic_oval_isNumeric(oval_t* oval);
uint8_t         otic_oval_cmp(const oval_t* val1, const oval_t* val2);
void            otic_oval_cpy(oval_t* dest, const oval_t* source);
otic_type_e     otic_oval_getType(const oval_t* val);

uint8_t         oval_array_cmp(const oval_array_t* ovalArray1, const oval_array_t* ovalArray2);

uint8_t         leb128_encode_unsigned(uint64_t value, uint8_t* restrict dest) NONNULL(2);
uint8_t         leb128_decode_unsigned(const uint8_t* restrict encoded_values, uint64_t* restrict value) NONNULL(1, 2);
uint8_t         leb128_encode_signed(int64_t value, uint8_t* restrict dest) NONNULL(2);
uint8_t         leb128_decode_signed(const uint8_t* restrict encoded_values, int64_t* restrict value) NONNULL(1, 2);

#ifdef __cplusplus
}
#endif

#endif //OTIC_BASE_H
