//
// Created by hp on 9/18/19.
//

#ifndef OTIC_PACK_H
#define OTIC_PACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zstd.h>
#include "config/config.h"
#include "core/core.h"


#define ZSTD_OUT_SIZE 1024
#define OTIC_PACK_CACHE_TOP_LIMIT 255
#define OTIC_PACK_NO_COMPRESSION 0


typedef struct otic_entry_t otic_entry_t;
struct otic_entry_t
{
    uint32_t index;
    char* name;
    union
    {
        uint32_t int_value;
        double double_value;
        struct
        {
            char* value;
            size_t size;
        } string_value;
    } last_value;
    otic_types_e type;
    otic_entry_t* next;
};

typedef struct otic_pack_t otic_pack_t;
typedef struct
{
    otic_base_t base;
    ZSTD_CCtx* cCtx;
    otic_entry_t* cache[OTIC_PACK_CACHE_SIZE];
    uint16_t totalEntries;
    uint8_t ztd_out[ZSTD_OUT_SIZE];
    const uint8_t* threshold;
    struct
    {
        otic_pack_t* parent;
        channel_type_e channelType;
        uint8_t channelId;
    } info;
} otic_pack_channel_t;

uint8_t otic_pack_channel_init(otic_pack_channel_t* channel, uint8_t id, channel_type_e channelType, const char* metaData,
                                otic_pack_t* parent);
uint8_t otic_pack_channel_close(otic_pack_channel_t* channel);
uint8_t otic_pack_channel_inject_i(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* sensorUnit, uint32_t value);
uint8_t otic_pack_channel_inject_i_neg(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* sensorUnit, uint32_t value);
uint8_t otic_pack_channel_inject_d(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, double value);
uint8_t otic_pack_channel_inject_s(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, const char* value);
uint8_t otic_pack_channel_inject_n(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit);
uint8_t otic_pack_channel_inject_b(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, uint8_t* buffer, size_t size);
uint8_t otic_pack_channel_flush(otic_pack_channel_t* channel);


struct otic_pack_t
{
    otic_pack_channel_t** channels;
    uint8_t totalChannels;
    uint8_t (*flusher)(uint8_t *, size_t);
    otic_errors_e error;
    otic_state_e state;
};

uint8_t otic_pack_init(otic_pack_t* oticPack, uint8_t(*flusher)(uint8_t*, size_t));

otic_pack_channel_t*    otic_pack_defineChannel(otic_pack_t* oticPack, channel_type_e channelType, uint8_t id,
                                                otic_features_e features) __attribute__((warn_unused_result));
uint8_t                 otic_pack_closeChannel(otic_pack_t* oticPackBase, uint8_t id);
uint8_t                 otic_pack_flush(otic_pack_t* oticPack);
void                    otic_pack_close(otic_pack_t* oticPack);

#ifdef __cpluscplus
}
#endif

#endif //OTIC_PACK_H