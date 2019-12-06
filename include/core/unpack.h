#ifndef OTIC_UNPACK_H
#define OTIC_UNPACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zstd.h>
#include "base.h"

#define OTIC_UNPACK_OUT_SIZE 12000
#define OTIC_UNPACK_OUTPUT_LIMIT 512
#define OTIC_UNPACK_RESULT_OUTSIZE 1048576
#if OTIC_UNPACK_RESULT_OUTSIZE < (OTIC_UNPACK_OUTPUT_LIMIT * 2)
#error OTIC unpack requires the Output buffer to be at least twice as big as the threshold limit
#endif
#define OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE 8


typedef struct
{
    uint32_t index;
    char* name;
    char* unit;
    oval_t value;
    uint8_t ignore;
} oticUnpackEntry_t;

typedef struct otic_unpack_t otic_unpack_t;

typedef struct
{
    otic_base_t base;
    ZSTD_DCtx* dCtx;
    struct {
        oticUnpackEntry_t** cache;
        size_t totalEntries;
        uint8_t allocationLeft;
        size_t cache_allocated;
        oticUnpackEntry_t* currentEntry;
    } cache;
    double ts;
    uint8_t out[OTIC_UNPACK_OUT_SIZE];
    uint8_t(*flusher)(double, const char*, const char*, const oval_t*, void*);
    void* data;
    struct
    {
        otic_unpack_t *parent;
        channel_type_e channelType;
        uint8_t channelId;
        otic_meta_data_t* metaData;
    } info;
    uint32_t entryIndex;
    size_t blockSize;
    struct
    {
        size_t* ptr;
        size_t size;
    } toFetch;
} oticUnpackChannel_t;

uint8_t                 otic_unpack_channel_init(oticUnpackChannel_t* channel, uint8_t id, uint8_t(*flusher)(double, const char*, const char*, const oval_t*, void* data),void* data, otic_unpack_t* parent);
void                    otic_unpack_channel_toFetch(oticUnpackChannel_t* channel, const char** values, size_t size);
uint8_t                 otic_unpack_channel_close(oticUnpackChannel_t* channel);

struct otic_unpack_t
{
    oticUnpackChannel_t** channels;
    uint8_t totalChannels;
    otic_errors_e error;
    otic_state_e state;
    void* fetcherData, *seekerData;
    uint8_t(*fetcher)(uint8_t*, size_t, void*);
    uint8_t(*seeker)(uint32_t, void*);
};

uint8_t                 otic_unpack_init(otic_unpack_t* oticUnpack, uint8_t(*fetcher)(uint8_t*, size_t, void*), void* fetcherData, uint8_t(*seeker)(uint32_t, void*), void* seekerData) __attribute__((nonnull(1,2)));
oticUnpackChannel_t*    otic_unpack_defineChannel(otic_unpack_t* oticUnpack, uint8_t id, uint8_t(*flusher)(double, const char*, const char*, const oval_t*, void* data), void* data) __attribute__((nonnull(1, 3)));
uint8_t                 otic_unpack_closeChannel(otic_unpack_t* oticUnpack,uint8_t id) __attribute__((nonnull(1)));
uint8_t                 otic_unpack_parse(otic_unpack_t* oticUnpackBase) __attribute__((nonnull(1)));
uint8_t                 otic_unpack_close(otic_unpack_t* oticUnpackBase) __attribute__((nonnull(1)));

#ifdef __cplusplus
}
#endif

#endif //OTIC_UNPACK_H