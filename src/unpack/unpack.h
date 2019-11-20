#ifndef OTIC_UNPACK_H
#define OTIC_UNPACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zstd.h>
#include "core/core.h"

#define OTIC_UNPACK_OUT_SIZE 12000
#define OTIC_UNPACK_OUTPUT_LIMIT 512
#define OTIC_UNPACK_RESULT_OUTSIZE 1048576
#if OTIC_UNPACK_RESULT_OUTSIZE < (OTIC_UNPACK_OUTPUT_LIMIT * 2)
#error OTIC unpack requires the Output buffer to be twice as big as the threshold limit
#endif
#define OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE 8

typedef struct
{
    uint32_t index;
    char* name;
    char* unit;
    union{
        uint32_t int_value;
        double double_value;
        struct
        {
            char* value;
            size_t size;
        } string_value;
    } last_value;
    otic_types_e type;
} otic_unpack_entry_t;


typedef struct otic_unpack_t otic_unpack_t;

typedef struct
{
    otic_base_t base;
    ZSTD_DCtx* dCtx;
    struct {
        otic_unpack_entry_t** cache;
        size_t totalEntries;
        uint8_t allocationLeft;
        size_t cache_allocated;
        otic_unpack_entry_t* currentEntry;
    } cache_t;
    struct{
        char content[OTIC_UNPACK_RESULT_OUTSIZE];
        char* top;
        char* limit;
    } result;
    uint8_t out[OTIC_UNPACK_OUT_SIZE];
    uint8_t(*flusher)(uint8_t*, size_t, void*);
    size_t blockSize;
    double doubleTs;
    void* data;
    struct
    {
        otic_unpack_t *parent;
        channel_type_e channelType;
        uint8_t channelId;
        otic_meta_data_t* metaData;
    } info;
    uint32_t entryIndex;
} otic_unpack_channel_t;

// TODO: ADD A GETINFO() FUNCTIONALITY IN EACH BINDING
struct otic_unpack_t
{
    otic_unpack_channel_t** channels;
    uint8_t totalChannels;
    otic_errors_e error;
    otic_state_e state;
    void* fetcherData, *seekerData;
    uint8_t(*fetcher)(uint8_t*, size_t, void*);
    uint8_t(*seeker)(uint32_t, void*);
};

uint8_t otic_unpack_init(otic_unpack_t* oticUnpack, uint8_t(*fetcher)(uint8_t*, size_t, void*), void* fetcherData, uint8_t(*seeker)(uint32_t, void*), void* seekerData) __attribute__((nonnull(1,2)));
uint8_t otic_unpack_defineChannel(otic_unpack_t* oticUnpack, uint8_t id, uint8_t(*flusher)(uint8_t*, size_t, void* data), void* data) __attribute__((nonnull(1, 3)));
uint8_t otic_unpack_closeChannel(otic_unpack_t* oticUnpack,uint8_t id);
uint8_t otic_unpack_parse(otic_unpack_t* oticUnpackBase);
uint8_t otic_unpack_close(otic_unpack_t* oticUnpackBase);

#ifdef __cplusplus
}
#endif

#endif //OTIC_UNPACK_H