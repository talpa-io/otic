#include "../core/core.h"
#include "pack.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <fenv.h>

#define OTIC_PACK_INLINE_ALL_STATIC 1

#if OTIC_PACK_INLINE_ALL_STATIC
#define OTIC_PACK_INLINE inline
#else
#define OTIC_PACK_INLINE
#endif


OTIC_PACK_INLINE
static uint32_t otic_hashFunction(const char* string)
{
    uint32_t hash_address = 0;
    uint8_t* ptr = (uint8_t*)string;
    while(*ptr)
    {
        hash_address = PTR_M * hash_address + *ptr;
        ptr++;
    }
    return hash_address % OTIC_PACK_CACHE_SIZE;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_find(otic_pack_t* pack, const char* o)
{
    otic_entry_t* ptr = pack->cache[otic_hashFunction(o)];
    while (ptr)
    {
        if (strcmp(ptr->name, o) == 0)
            return ptr;
        ptr = ptr->next;
    }
    return 0;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_routine(otic_pack_t* pack, const char* o, const char* unit, uint16_t index)
{
    uint32_t hash_address = otic_hashFunction(o);
    otic_entry_t* ptr = pack->cache[hash_address];
    while (ptr)
    {
        if (ptr->index == index)
            return ptr;
        ptr = ptr->next;
    }
    ptr = malloc(sizeof(otic_entry_t));
    if (!ptr)
        return 0;
    ptr->next = 0;
    ptr->last_value.string_value.value = 0;
    ptr->last_value.string_value.size = 0;
    uint8_t len[2] = {strlen(o), strlen(unit)};
    ptr->name = malloc(len[0] + len[1] + 1);
    strcpy(ptr->name, o);
    ptr->name[len[0] + 1] = ':';
    strcpy(ptr->name + len[0] + 2, unit);
    ptr->index = index;
    ptr->next = pack->cache[hash_address];
    pack->cache[hash_address] = ptr;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_u(otic_pack_t* pack, const char* o, const char* unit, uint16_t index, uint32_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(pack, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_INT32_POS;
    ptr->last_value.int_value = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_un(otic_pack_t* pack, const char* o, const char* unit, uint16_t index, uint32_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(pack, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_INT32_NEG;
    ptr->last_value.int_value = value;
    return ptr;
}


OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_d(otic_pack_t* pack, const char* o, const char* unit, uint16_t index, double value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(pack, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_DOUBLE;
    ptr->last_value.double_value = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_s(otic_pack_t* pack, const char* o, const char* unit, uint16_t index, const char* value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(pack, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_STRING;
    ptr->last_value.string_value.size = strlen(value);
    ptr->last_value.string_value.value = malloc(ptr->last_value.string_value.size + 1);
    strcpy(ptr->last_value.string_value.value, value);
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_n(otic_pack_t* pack, const char* o, const char* unit, uint16_t index){
    otic_entry_t* ptr = otic_pack_entry_insert_routine(pack, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_NULL;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_b(otic_pack_t* pack, const char* o, const char* unit, uint16_t index)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(pack, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_RAWBUFFER;
    return ptr;
}

OTIC_PACK_INLINE
static void otic_pack_write_byte(otic_pack_t* oticPack, uint8_t value)
{
    *oticPack->base.top++ = value;
}

OTIC_PACK_INLINE
void static write_long(otic_pack_t* oticPack, long value)
{
    oticPack->base.top += leb128_encode_unsigned(value, oticPack->base.top);
}

OTIC_PACK_INLINE
static void write_double(otic_pack_t* oticPack, double value)
{
    memcpy(oticPack->base.top, (char*)&value, sizeof(double));
    oticPack->base.top += sizeof(double);
}

OTIC_PACK_INLINE
static void write_string(otic_pack_t* oticPack, const char* value, uint8_t len)
{
    otic_pack_write_byte(oticPack, len);
    memcpy(oticPack->base.top, value, len);
    oticPack->base.top += len;
}

OTIC_PACK_INLINE
static void write_buffer(otic_pack_t* oticPack, uint8_t* buffer, size_t size)
{
    memcpy(oticPack->base.top, (char*)&size, sizeof(size));
    oticPack->base.top += sizeof(size);
    memcpy(oticPack->base.top, buffer, size);
    oticPack->base.top += size;
}

OTIC_PACK_INLINE
void static otic_pack_id_assign(otic_pack_t* oticPack, const char* sensorName, const char* unit)
{
    otic_pack_write_byte(oticPack, OTIC_TYPE_NAME_ASSIGN);
    uint8_t len[2] = {strlen(sensorName), strlen(unit)};
    otic_pack_write_byte(oticPack, len[0] + len[1] + 1);
    memcpy(oticPack->base.top, sensorName, len[0]);
    oticPack->base.top += len[0];
    otic_pack_write_byte(oticPack, ':');
    memcpy(oticPack->base.top, unit, len[1]);
    oticPack->base.top += len[1];
}

OTIC_PACK_INLINE
uint8_t static otic_ts_handler(otic_pack_t* oticPack, double ts)
{
    static uint64_t intTs;
    intTs = (uint64_t)(ts * OTIC_TS_MULTIPLICATOR);
    if (oticPack->base.timestamp_current == ts){
            return 1;
    } else if (oticPack->base.timestamp_current == 0){
        oticPack->base.timestamp_start = oticPack->base.timestamp_current = intTs;
        otic_pack_write_byte(oticPack, OTIC_TYPE_SET_TIMESTAMP);
        write_long(oticPack, intTs);
    } else {
        if (ts < oticPack->base.timestamp_current){
            otic_base_setError(&oticPack->base, OTIC_ERROR_INVALID_TIMESTAMP);
            return 0;
        }
        otic_pack_write_byte(oticPack, OTIC_TYPE_SHIFT_TIMESTAMP);
        write_long(oticPack, intTs - oticPack->base.timestamp_current);
        oticPack->base.timestamp_current = intTs;
    }
    return 1;
}

OTIC_PACK_INLINE
void static otic_metadata_write(otic_pack_t* oticPack, const char* value)
{
    otic_pack_write_byte(oticPack, OTIC_TYPE_METADATA);
    write_string(oticPack, value, strlen(value));
}

OTIC_PACK_INLINE
static uint8_t otic_pack_flush(otic_pack_t* oticPack)
{
#if OTIC_PACK_NO_COMPRESSION
    if (!oticPack->flusher[0](oticPack->base.cache, oticPack->base.top - oticPack->base.cache)){
        otic_base_setError(&oticPack->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
#else
    size_t ret = ZSTD_compressCCtx(oticPack->cCtx, oticPack->zstd_out, ZSTD_OUT_SIZE, oticPack->base.cache, oticPack->base.top - oticPack->base.cache, 1);
    if (ZSTD_isError(ret)){
        otic_base_setError(&oticPack->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
    if (!oticPack->flusher((uint8_t*)&ret, sizeof(uint32_t))){
        otic_base_setError(&oticPack->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
    if (!oticPack->flusher(oticPack->zstd_out, ret)){
        otic_base_setError(&oticPack->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
#endif
    oticPack->base.top = oticPack->base.cache;
    return 1;
fail:
    otic_base_setState(&oticPack->base, OTIC_STATE_ON_ERROR);
    return 0;
}

OTIC_PACK_INLINE
void static otic_pack_flush_if_flushable(otic_pack_t* oticPack)
{
    if (oticPack->base.top > oticPack->limit)
        otic_pack_flush(oticPack);
}

OTIC_PACK_INLINE
static uint8_t otic_pack_init(otic_pack_t* oticPack, uint8_t(*flusher)(uint8_t* buffer, size_t size), const char* metadata)
{
    if (!otic_base_init(&oticPack->base))
        goto fail;
    if (!flusher){
        otic_base_setError(&oticPack->base, OTIC_ERROR_INVALID_POINTER);
        goto fail;
    }
    oticPack->cCtx = ZSTD_createCCtx();
    if (!oticPack->cCtx){
        otic_base_setError(&oticPack->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
//#pragma STDC FENV_ACCESS ON
//    fesetround(FE_UPWARD);
    size_t counter;
    oticPack->flusher = flusher;
    for (counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
        oticPack->cache[counter] = 0;
    oticPack->totalEntries = 0;
    oticPack->limit = &oticPack->base.cache[OTIC_BASE_CACHE_SIZE - OTIC_PACK_CACHE_TOP_LIMIT];
    otic_header_t oticHeader = {.features = 1, .version = OTIC_VERSION_MAJOR};
    oticHeader.magic[0] = 'o';
    oticHeader.magic[1] = 'c';
    oticHeader.magic[2] = 0x07;
    oticHeader.magic[3] = 0xFF;
    flusher((uint8_t*)&oticHeader, sizeof(otic_header_t));
    // TODO: Implement MetaData
    return 1;
fail:
    otic_base_setState(&oticPack->base, OTIC_STATE_ON_ERROR);
    return 0;
}
OTIC_PACK_INLINE
static uint8_t otic_pack_inject_in(otic_pack_t *oticPack, double timestamp, const char *sensorName, const char *unit, int64_t value) {
    otic_ts_handler(oticPack, timestamp);
    otic_entry_t *entry = otic_pack_entry_find(oticPack, sensorName);
    if (!entry){
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&oticPack->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_un(oticPack, sensorName, unit, oticPack->totalEntries, value)){
            otic_base_setError(&oticPack->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(oticPack, sensorName, unit);
        otic_pack_write_byte(oticPack, OTIC_TYPE_INT32_NEG);
        write_long(oticPack, oticPack->totalEntries);
        write_long(oticPack, value);
        oticPack->totalEntries++;
    } else{
        if (entry->last_value.int_value == value && entry->type == OTIC_TYPE_INT32_NEG){
            otic_pack_write_byte(oticPack, OTIC_TYPE_UNMODIFIED);
            write_long(oticPack, entry->index);
        } else {
            entry->last_value.int_value = value;
            otic_pack_write_byte(oticPack, OTIC_TYPE_INT32_NEG);
            write_long(oticPack, entry->index);
            write_long(oticPack, value);
        }
    }
    oticPack->base.rowCounter++;
    otic_pack_flush_if_flushable(oticPack);
    return 1u;
fail:
    otic_base_setState(&oticPack->base, OTIC_STATE_CLOSED);
    return 0;
}

OTIC_PACK_INLINE
static uint8_t otic_pack_inject_i(otic_pack_t *oticPack, double timestamp, const char *sensorName, const char *unit, int64_t value) {
    otic_ts_handler(oticPack, timestamp);
    otic_entry_t *entry = otic_pack_entry_find(oticPack, sensorName);
    if (!entry){
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&oticPack->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_u(oticPack, sensorName, unit, oticPack->totalEntries, value)){
            otic_base_setError(&oticPack->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(oticPack, sensorName, unit);
        otic_pack_write_byte(oticPack, OTIC_TYPE_INT32_POS);
        write_long(oticPack, oticPack->totalEntries);
        write_long(oticPack, value);
        oticPack->totalEntries++;
    } else{
        if (entry->last_value.int_value == value){
            otic_pack_write_byte(oticPack, OTIC_TYPE_UNMODIFIED);
            write_long(oticPack, entry->index);
        } else {
            entry->last_value.int_value = value;
            otic_pack_write_byte(oticPack, OTIC_TYPE_INT32_POS);
            write_long(oticPack, entry->index);
            write_long(oticPack, value);
        }
    }
    oticPack->base.rowCounter++;
    otic_pack_flush_if_flushable(oticPack);
    return 1u;
fail:
    otic_base_setState(&oticPack->base, OTIC_STATE_CLOSED);
    return 0;
}

OTIC_PACK_INLINE
static uint8_t otic_pack_inject_d(otic_pack_t* oticPack, double timestamp, const char* sensorName, const char* unit, double value)
{
    otic_ts_handler(oticPack, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(oticPack, sensorName);
    if (!entry){
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&oticPack->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_d(oticPack, sensorName, unit, oticPack->totalEntries, value);
        otic_pack_id_assign(oticPack, sensorName, unit);
        otic_pack_write_byte(oticPack, OTIC_TYPE_DOUBLE);
        write_long(oticPack, oticPack->totalEntries);
        write_double(oticPack, value);
        oticPack->totalEntries++;
    } else {
        if (entry->last_value.double_value == value) {
            otic_pack_write_byte(oticPack, OTIC_TYPE_UNMODIFIED);
            write_long(oticPack, entry->index);
        } else {
            entry->last_value.double_value = value;
            otic_pack_write_byte(oticPack, OTIC_TYPE_DOUBLE);
            write_long(oticPack, entry->index);
            write_double(oticPack, value);
        }
    }
    oticPack->base.rowCounter++;
    otic_pack_flush_if_flushable(oticPack);
    return 1u;
fail:
    otic_base_setState(&oticPack->base, OTIC_STATE_ON_ERROR);
    return 0;
}

OTIC_PACK_INLINE
static uint8_t otic_pack_inject_s(otic_pack_t* oticPack, double timestamp, const char* sensorName, const char* unit, const char* value)
{
    otic_ts_handler(oticPack, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(oticPack, sensorName);
    size_t len1 = strlen(value);
    if (len1 > UINT8_MAX){
        otic_base_setError(&oticPack->base, OTIC_ERROR_BUFFER_OVERFLOW);
        goto fail;
    }
    if (!entry){
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&oticPack->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_s(oticPack, sensorName, unit, oticPack->totalEntries, value);
        otic_pack_id_assign(oticPack, sensorName, unit);
        otic_pack_write_byte(oticPack, OTIC_TYPE_STRING);
        write_long(oticPack, oticPack->totalEntries);
        write_string(oticPack, value, len1);
        oticPack->totalEntries++;
    } else {
        if (strcmp(entry->last_value.string_value.value, value) == 0){
            otic_pack_write_byte(oticPack, OTIC_TYPE_UNMODIFIED);
            write_long(oticPack, entry->index);
        } else {
            if (entry->last_value.string_value.size < strlen(value))
                entry->last_value.string_value.value = realloc(entry->last_value.string_value.value, len1 + 1);
            strcpy(entry->last_value.string_value.value, value);
            otic_pack_write_byte(oticPack, OTIC_TYPE_STRING);
            write_long(oticPack, entry->index);
            write_string(oticPack, value, len1);
        }
    }
    oticPack->base.rowCounter++;
    otic_pack_flush_if_flushable(oticPack);
    return 1u;
fail:
    otic_base_setState(&oticPack->base, OTIC_STATE_ON_ERROR);
    return 0;
}

OTIC_PACK_INLINE
static uint8_t otic_pack_inject_n(otic_pack_t* oticPack, double timestamp, const char* sensorName, const char* unit)
{
    otic_ts_handler(oticPack, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(oticPack, sensorName);
    if (!entry) {
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&oticPack->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_n(oticPack, sensorName, unit, oticPack->totalEntries)){
            otic_base_setError(&oticPack->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(oticPack, sensorName, unit);
        otic_pack_write_byte(oticPack, OTIC_TYPE_NULL);
        write_long(oticPack, oticPack->totalEntries);
        oticPack->totalEntries++;
    } else{
        if (entry->type == OTIC_TYPE_NULL) {
            otic_pack_write_byte(oticPack, OTIC_TYPE_UNMODIFIED);
            write_long(oticPack, entry->index);
        } else {
            entry->type = OTIC_TYPE_NULL;
            otic_pack_write_byte(oticPack, OTIC_TYPE_NULL);
        }
    }
    oticPack->base.rowCounter++;
    otic_pack_flush_if_flushable(oticPack);
    return 1;
fail:
    otic_base_setState(&oticPack->base, OTIC_STATE_CLOSED);
    return 0;
}

OTIC_PACK_INLINE
static uint8_t otic_pack_inject_b(otic_pack_t* oticPack, double timestamp, const char* sensorName, const char* unit, uint8_t* buffer, size_t size)
{
    otic_ts_handler(oticPack, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(oticPack, sensorName);
    if (!entry){
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&oticPack->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_b(oticPack, sensorName, unit, oticPack->totalEntries)){
            otic_base_setError(&oticPack->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(oticPack, sensorName, unit);
        otic_pack_write_byte(oticPack, OTIC_TYPE_RAWBUFFER);
        write_long(oticPack, oticPack->totalEntries);
        oticPack->totalEntries++;
    } else {
        entry->type = OTIC_TYPE_RAWBUFFER;
        otic_pack_write_byte(oticPack, OTIC_TYPE_RAWBUFFER);
        write_long(oticPack, entry->index);
    }
    write_buffer(oticPack, buffer, size);
    oticPack->base.rowCounter++;
    otic_pack_flush_if_flushable(oticPack);
    return 1u;
fail:
    otic_base_setState(&oticPack->base, OTIC_STATE_CLOSED);
    return 0;
}

OTIC_PACK_INLINE
static uint8_t otic_pack_close(otic_pack_t* oticPack)
{
    if (!oticPack)
        goto fail;
    otic_pack_write_byte(oticPack, OTIC_TYPE_EOF);
    write_long(oticPack, oticPack->base.rowCounter);
    otic_pack_flush(oticPack);
    ZSTD_freeCCtx(oticPack->cCtx);
    size_t counter;
    for (counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
    {
        if (oticPack->cache[counter] != 0)
        {
            free(oticPack->cache[counter]->name);
            if (oticPack->cache[counter]->last_value.string_value.size)
                free(oticPack->cache[counter]->last_value.string_value.value);
            // TODO: Free
//            free(oticPack->cache[counter]);
//            otic_entry_t* ptr = oticPack->cache[counter];
//            otic_entry_t* next = 0;
//            while(ptr != 0)
//            {
//                next = ptr->next;
//                free(ptr);
//                ptr = next;
//            }
//            free(oticPack->cache[counter]);
        }
    }
    return 1;
fail:
    return 0;
}


// ---------------------------------------------------------------------------------------------------------------------



uint8_t otic_pack_channel_init(channel_t* channel, uint32_t id, channel_type_e channelType, uint8_t (*flusher)(uint8_t*, size_t), const char* metaData, otic_pack_base_t* parent)
{
    if (!channel)
        return 0;
    if (!otic_pack_init(&channel->oticPack, flusher, ""))
        goto fail;
    channel->channelId = id;
    channel->channelType = channelType;
    channel->parent = parent;
    return 1;
fail:
    return 0;
}

uint8_t otic_pack_channel_close(channel_t* channel)
{
    if (!channel)
        goto fail;
    if (!otic_pack_close(&channel->oticPack))
        goto fail;
    // TODO: IMPLEMENT CLOSE
    return 1;
fail:
    return 0;
}

uint8_t otic_pack_channel_inject_i(channel_t* channel, double timestamp, const char* sensorName, const char* sensorUnit, uint32_t value)
{
    return otic_pack_inject_i(&channel->oticPack, timestamp, sensorName, sensorUnit, value);
}

uint8_t otic_pack_channel_inject_i_neg(channel_t* channel, double timestamp, const char* sensorName, const char* sensorUnit, uint32_t value)
{
    return otic_pack_inject_in(&channel->oticPack, timestamp, sensorName, sensorUnit, value);
}

uint8_t otic_pack_channel_inject_d(channel_t* channel, double timestamp, const char* sensorName, const char* unit, double value)
{
    return otic_pack_inject_d(&channel->oticPack, timestamp, sensorName, unit, value);
}

uint8_t otic_pack_channel_inject_s(channel_t* channel, double timestamp, const char* sensorName, const char* unit, const char* value)
{
    return otic_pack_inject_s(&channel->oticPack, timestamp, sensorName, unit, value);
}

uint8_t otic_pack_channel_inject_n(channel_t* channel, double timestamp, const char* sensorName, const char* unit)
{
    return otic_pack_inject_n(&channel->oticPack, timestamp, sensorName, unit);
}

uint8_t otic_pack_channel_inject_b(channel_t* channel, double timestamp, const char* sensorName, const char* unit, uint8_t* buffer, size_t size)
{
    return otic_pack_inject_b(&channel->oticPack, timestamp, sensorName, unit, buffer, size);
}

uint8_t otic_pack_channel_flush(channel_t* channel)
{
    return otic_pack_flush(&channel->oticPack);
}

void otic_pack_base_init(otic_pack_base_t* oticPackBase)
{
    if (!oticPackBase)
        return;
    oticPackBase->channels = 0;
    oticPackBase->totalChannels = 0;
}

channel_t* otic_pack_base_defineChannel(otic_pack_base_t* oticPackBase, channel_type_e channelType, uint32_t id, uint8_t(*flusher)(uint8_t*, size_t))
{
    if (!oticPackBase || id > UINT32_MAX)
        return 0;
    oticPackBase->channels = realloc(oticPackBase->channels, sizeof(channel_t*) * (oticPackBase->totalChannels + 1));
    if (!oticPackBase->channels)
        return 0;
    oticPackBase->channels[oticPackBase->totalChannels] = malloc(sizeof(channel_t));
    if (!oticPackBase->channels[oticPackBase->totalChannels])
        return 0;
    otic_pack_channel_init(oticPackBase->channels[oticPackBase->totalChannels], id, channelType, flusher, "", oticPackBase);
    return oticPackBase->channels[oticPackBase->totalChannels++];
}

uint8_t otic_pack_base_closeChannel(otic_pack_base_t* oticPackBase, uint32_t id)
{
    uint32_t counter;
    for (counter = 0; counter < oticPackBase->totalChannels; counter++)
    {
        if (oticPackBase->channels[counter]->channelId == id)
        {
            otic_pack_channel_close(oticPackBase->channels[counter]);
            channel_t* temp = oticPackBase->channels[counter];
            oticPackBase->channels[counter] = oticPackBase->channels[oticPackBase->totalChannels];
            free(temp);
            oticPackBase = realloc(oticPackBase, sizeof(channel_t*) * --oticPackBase->totalChannels);
            return 1;
        }
    }
    return 0;
}

void otic_pack_base_close(otic_pack_base_t* oticPackBase)
{
    if (!oticPackBase)
        return;
    uint32_t counter = 0;
    for (counter = 0; counter < oticPackBase->totalChannels; counter++)
    {
        free(oticPackBase->channels[counter]);
    }
    free(oticPackBase->channels);
}