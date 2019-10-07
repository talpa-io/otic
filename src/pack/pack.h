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
#define OTIC_PACK_BAUDWIDTH 3
#define OTIC_PACK_NO_COMPRESSION 0


typedef struct otic_entry_t otic_entry_t;
struct otic_entry_t
{
    uint32_t index;
    char* name;
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
    otic_entry_t* next;
};

typedef struct
{
    otic_base_t base;
    ZSTD_CCtx* cCtx;
    otic_entry_t* cache[OTIC_PACK_CACHE_SIZE];
    uint16_t totalEntries;
    uint8_t zstd_out[ZSTD_OUT_SIZE];
    uint8_t(*flusher[OTIC_PACK_BAUDWIDTH])(uint8_t*, size_t);
    const uint8_t* limit;
} otic_pack_t;

uint8_t otic_pack_init(otic_pack_t* oticPack, uint8_t(*flusher)(uint8_t* buffer, size_t size), const char* metadata);
uint8_t otic_pack_close(otic_pack_t* oticPack);
uint8_t otic_pack_inject_i(otic_pack_t *oticPack, double timestamp, const char *sensorName, const char *unit, int64_t value);
uint8_t otic_pack_inject_d(otic_pack_t* oticPack, double timestamp, const char* sensorName, const char* unit, double value);
uint8_t otic_pack_inject_s(otic_pack_t* oticPack, double timestamp, const char* sensorName, const char* unit, const char* value);
uint8_t otic_pack_inject_n(otic_pack_t* oticPack, double timestamp, const char* sensorName, const char* unit);
uint8_t otic_pack_inject_b(otic_pack_t* oticPack, double timestamp, const char* sensorName, const char* unit, uint8_t* buffer, size_t size);
uint8_t otic_pack_flush(otic_pack_t* oticPack);


uint8_t otic_pack_setChannel(otic_pack_t* oticPack, uint8_t channelNumber, uint8_t(*flusher)(uint8_t*, size_t));
void    otic_pack_clearChannel(otic_pack_t* oticPack, uint8_t channelNumber);


#ifdef __cpluscplus
}
#endif

#endif //OTIC_PACK_H