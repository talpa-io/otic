#ifndef OTIC_PACK_H
#define OTIC_PACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zstd.h>
#include "config.h"
#include "base.h"

#define OTIC_PACK_CACHE_TOP_LIMIT 255

typedef struct otic_entry_t otic_entry_t;
typedef void (*ovalClean)(oval_t*);
struct otic_entry_t
{
    uint32_t index;
    char* name;
    oval_t lastValue;
    otic_entry_t* next;
};

typedef struct otic_pack_t otic_pack_t;
typedef struct otic_pack_channel_t otic_pack_channel_t;

struct otic_pack_channel_t
{
    otic_base_t base;
    ZSTD_CCtx* cCtx;
    otic_entry_t* cache[OTIC_PACK_CACHE_SIZE];
    uint16_t totalEntries;
    uint8_t* ztd_out;
    const uint8_t* threshold;
    struct
    {
        otic_pack_t* parent;
        channel_type_e channelType;
        uint8_t channelId;
    } info;
    time_interval_t timeInterval;
    otic_pack_channel_t* previous;
#ifdef OTIC_STATS
    otic_sta
#endif
};

uint8_t otic_pack_channel_init(
        otic_pack_channel_t* channel,
        uint8_t id,
        channel_type_e channelType,
        otic_pack_t* parent, uint32_t bucketSize
        ) NONNULL(1);
uint8_t otic_pack_channel_close(otic_pack_channel_t* channel) NONNULL(1);

#ifdef bool
#define BOOL_UINT8 bool
#else
#define BOOL_UINT8 uint8_t
#endif
uint8_t otic_pack_channel_inject_bool(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* sensorUnit, BOOL_UINT8 value);
uint8_t otic_pack_channel_inject_i(
        otic_pack_channel_t* channel,
        double timestamp,
        const char* sensorName,
        const char* sensorUnit,
        uint64_t value
        ) NONNULL(1);
uint8_t otic_pack_channel_inject_i_neg(
        otic_pack_channel_t* channel,
        double timestamp,
        const char* sensorName,
        const char* sensorUnit,
        uint64_t value
        ) NONNULL(1);
uint8_t otic_pack_channel_inject_d(
        otic_pack_channel_t* channel,
        double timestamp,
        const char* sensorName,
        const char* unit,
        double value
        ) NONNULL(1);
uint8_t otic_pack_channel_inject_s(
        otic_pack_channel_t* channel,
        double timestamp,
        const char* sensorName,
        const char* unit,
        const char* value
        ) NONNULL(1);
uint8_t otic_pack_channel_inject_n(
        otic_pack_channel_t* channel,
        double timestamp,
        const char* sensorName,
        const char* unit
        ) NONNULL(1);
uint8_t otic_pack_channel_inject_b(
        otic_pack_channel_t* channel,
        double timestamp,
        const char* sensorName,
        const char* unit,
        uint8_t* buffer,
        size_t size
        ) NONNULL(1);

uint8_t otic_pack_channel_flush(otic_pack_channel_t* channel) NONNULL(1);
uint8_t otic_pack_channel_inject_array(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, const oval_array_t* v);
uint8_t otic_pack_channel_resizeBucket(otic_pack_channel_t* channel, uint32_t bucketSize);

struct otic_pack_t
{
    otic_pack_channel_t* channels;
    uint8_t (*flusher)(uint8_t *, size_t, void*);
    void* data;
    otic_error_e error;
    otic_state_e state;
};

uint8_t                 otic_pack_init(otic_pack_t* oticPack, uint8_t features, uint8_t(*flusher)(uint8_t*, size_t, void*), void* data) NONNULL(1);
otic_pack_channel_t*    otic_pack_defineChannel(otic_pack_t* oticPack, channel_type_e channelType, uint8_t id,
                                                otic_feature_e features, uint32_t bucketSize) __attribute__((warn_unused_result)) NONNULL(1);
#define otic_pack_defineChannel_defaultSize(oticPack, channelType, id, features) otic_pack_defineChannel(oticPack, channelType, id, features, bucketSize, 0)
uint8_t                 otic_pack_closeChannel(otic_pack_t* oticPackBase, uint8_t id) NONNULL(1);
uint8_t                 otic_pack_getTotalAmountOfChannel(otic_pack_t* oticPack) NONNULL(1);
uint8_t                 otic_pack_flush(otic_pack_t* oticPack) NONNULL(1);
uint8_t                 otic_pack_close(otic_pack_t* oticPack) NONNULL(1);

#ifdef __cplusplus
}
#endif

#endif //OTIC_PACK_H
