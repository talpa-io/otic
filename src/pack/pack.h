//
// Created by hp on 9/18/19.
//

#ifndef OTIC_PACK_H
#define OTIC_PACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zstd.h>
#include "../../include/config/config.h"
#include "../core/core.h"

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

typedef struct
{
    otic_base_t base;
    ZSTD_CCtx* cCtx;
    otic_entry_t* cache[OTIC_PACK_CACHE_SIZE];
    uint16_t totalEntries;
    uint8_t zstd_out[ZSTD_OUT_SIZE];
    uint8_t(*flusher)(uint8_t*, size_t);
    const uint8_t* limit;
} otic_pack_t;

typedef enum {
    OTIC_PACK_CHANNEL_SENSOR,
    OTIC_PACK_CHANNEL_BINARY
} channel_type_e;

typedef struct otic_pack_base_t otic_pack_base_t;
typedef struct
{
    otic_pack_t oticPack;
    otic_pack_base_t* parent;
    channel_type_e channelType;
    uint32_t channelId;
} channel_t;

uint8_t otic_pack_channel_init(channel_t* channel, uint32_t id, channel_type_e channelType, uint8_t (*flusher)(uint8_t*, size_t), const char* metaData, otic_pack_base_t* parent);
uint8_t otic_pack_channel_close(channel_t* channel);
uint8_t otic_pack_channel_inject_i(channel_t* channel, double timestamp, const char* sensorName, const char* sensorUnit, uint32_t value);
uint8_t otic_pack_channel_inject_i_neg(channel_t* channel, double timestamp, const char* sensorName, const char* sensorUnit, uint32_t value);
uint8_t otic_pack_channel_inject_d(channel_t* channel, double timestamp, const char* sensorName, const char* unit, double value);
uint8_t otic_pack_channel_inject_s(channel_t* channel, double timestamp, const char* sensorName, const char* unit, const char* value);
uint8_t otic_pack_channel_inject_n(channel_t* channel, double timestamp, const char* sensorName, const char* unit);
uint8_t otic_pack_channel_inject_b(channel_t* channel, double timestamp, const char* sensorName, const char* unit, uint8_t* buffer, size_t size);
uint8_t otic_pack_channel_flush(channel_t* channel);


struct otic_pack_base_t
{
    channel_t** channels;
    uint32_t totalChannels;
};

void        otic_pack_base_init(otic_pack_base_t* oticPackBase);
channel_t*  otic_pack_base_defineChannel(otic_pack_base_t* oticPackBase, channel_type_e channelType, uint32_t id, uint8_t(*flusher)(uint8_t*, size_t));
uint8_t     otic_pack_base_closeChannel(otic_pack_base_t* oticPackBase, uint32_t id);
void        otic_pack_base_close(otic_pack_base_t* oticPackBase);


#ifdef __cpluscplus
}
#endif

#endif //OTIC_PACK_H