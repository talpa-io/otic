#include <string.h>
#include <stdlib.h>
#include <printf.h>
#include <stdio.h>
#include "core/pack.h"

/**
 * The purpose of the following 6-7 lines, is to inline each static functions defines in this file.
 * This allows the use of the concerned functions without calling them, as their body gets replaced
 * in each call, resulting in a performance improvement.
 * So if this improves performance, why is \a inline an option?
 * The cost to pay for aligning each static function, is that the executable/library file considerably
 * increases for each \a inline.
 *
 * It is therefore recommended to disable \a inline if the user intends to use this library on a device
 * with little program storage and for which the wholesome performance is of little value.
 */
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

// TODO: Optimize strlen
OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_find(otic_pack_channel_t* pack, const char* o)
{
    otic_entry_t* ptr = pack->cache[otic_hashFunction(o)];
    while (ptr)
    {
        if (strncmp(ptr->name, o, strlen(o)) == 0)
            return ptr;
        ptr = ptr->next;
    }
    return 0;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_routine(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index)
{
    uint32_t hash_address = otic_hashFunction(o->ptr);
    otic_entry_t* ptr = channel->cache[hash_address];
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
    ptr->last_value.string_value.ptr = 0;
    ptr->last_value.string_value.size = 0;
    ptr->name = malloc(o->size + unit->size + 1);
    memcpy(ptr->name, o->ptr, o->size);
    ptr->name[o->size] = ':';
    memcpy(ptr->name + o->size + 1, unit->ptr, unit->size);
    ptr->name[o->size + unit->size] = 0;
    ptr->index = index;
    ptr->next = channel->cache[hash_address];
    channel->cache[hash_address] = ptr;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_u(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, uint32_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_INT32_POS;
    ptr->last_value.int_value = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_un(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, uint32_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_INT32_NEG;
    ptr->last_value.int_value = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_d(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, double value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_DOUBLE;
    ptr->last_value.double_value = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_s(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, const otic_str_t* value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_STRING;
    ptr->last_value.string_value.size = value->size;
    ptr->last_value.string_value.ptr = malloc(ptr->last_value.string_value.size + 1);
    memcpy((char*)ptr->last_value.string_value.ptr, value->ptr, value->size);
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_n(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index){
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_NULL;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_b(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_RAWBUFFER;
    return ptr;
}

OTIC_PACK_INLINE
static void otic_pack_write_byte(otic_pack_channel_t* channel, uint8_t value)
{
    *channel->base.top++ = value;
}

OTIC_PACK_INLINE
static void write_long(otic_pack_channel_t* channel, long value)
{
    channel->base.top += leb128_encode_unsigned(value, channel->base.top);
}

OTIC_PACK_INLINE
static void write_double(otic_pack_channel_t* channel, double value)
{
    memcpy(channel->base.top, (char*)&value, sizeof(double));
    channel->base.top += sizeof(double);
}

OTIC_PACK_INLINE
static void write_string(otic_pack_channel_t* channel, otic_str_t* value)
{
    otic_pack_write_byte(channel, value->size);
    memcpy(channel->base.top, value->ptr, value->size);
    channel->base.top += value->size;
}

OTIC_PACK_INLINE
static void write_buffer(otic_pack_channel_t* channel, uint8_t* buffer, size_t size)
{
    memcpy(channel->base.top, (char*)&size, sizeof(size));
    channel->base.top += sizeof(size);
    memcpy(channel->base.top, buffer, size);
    channel->base.top += size;
}

OTIC_PACK_INLINE
static void otic_pack_id_assign(otic_pack_channel_t* channel, const otic_str_t* sensorName, const otic_str_t* unit)
{
    otic_pack_write_byte(channel, OTIC_TYPE_NAME_ASSIGN);
    otic_pack_write_byte(channel, sensorName->size + unit->size + 1);
    memcpy(channel->base.top, sensorName->ptr, sensorName->size);
    channel->base.top += sensorName->size;
    otic_pack_write_byte(channel, ':');
    memcpy(channel->base.top, unit->ptr, unit->size);
    channel->base.top += unit->size;
}

/*OTIC_PACK_INLINE
uint8_t static otic_ts_handler(otic_pack_channel_t* channel, double ts)
{
    static uint64_t intTs;
    intTs = (uint64_t)(ts * OTIC_TS_MULTIPLICATOR);
    if (channel->base.timestamp_current == ts){
            return 1;
    } else if (channel->base.timestamp_current == 0){
        channel->base.timestamp_start = channel->base.timestamp_current = intTs;
        otic_pack_write_byte(channel, OTIC_TYPE_SET_TIMESTAMP);
        write_long(channel, intTs);
    } else {
        if (ts < channel->base.timestamp_current){
            otic_base_setError(&channel->base, OTIC_ERROR_INVALID_TIMESTAMP);
            return 0;
        }
        otic_pack_write_byte(channel, OTIC_TYPE_SHIFT_TIMESTAMP);
        write_long(channel, intTs - channel->base.timestamp_current);
        channel->base.timestamp_current = intTs;
    }
    return 1;
}*/

OTIC_PACK_INLINE
static uint8_t otic_ts_handler(otic_pack_channel_t* channel, double ts)
{
    static uint64_t intTs;
    intTs = (uint64_t) (ts * OTIC_TS_MULTIPLICATOR);

    if (channel->base.timestamp_start == 0)
        channel->base.timestamp_start = channel->base.timestamp_current = intTs;
    else if (channel->base.timestamp_current == intTs)
        return 1;
    else {

        if (intTs < channel->base.timestamp_current) {
            otic_base_setError(&channel->base, OTIC_ERROR_INVALID_TIMESTAMP);
            return 0;
        }
        otic_pack_write_byte(channel, OTIC_TYPE_SHIFT_TIMESTAMP);
        write_long(channel, intTs - channel->base.timestamp_current);
        channel->base.timestamp_current = intTs;
    }
    return 1;
}

OTIC_PACK_INLINE
static void otic_pack_flush_if_flushable(otic_pack_channel_t* channel)
{
    if (channel->base.top > channel->threshold)
        otic_pack_channel_flush(channel);
}

OTIC_PACK_INLINE
static void otic_pack_setError(otic_pack_t* restrict oticPack, otic_errors_e error)
{
    oticPack->error = error;
}

OTIC_PACK_INLINE
static otic_errors_e otic_pack_getError(otic_pack_t* restrict oticPack)
{
    return oticPack->error;
}

OTIC_PACK_INLINE
static void otic_pack_setState(otic_pack_t* restrict oticPack, otic_state_e state)
{
    oticPack->state = state;
}

OTIC_PACK_INLINE
static otic_state_e otic_pack_getState(otic_pack_t* restrict oticPack)
{
    return oticPack->state;
}

uint8_t otic_pack_channel_inject_i_neg(otic_pack_channel_t* channel, double timestamp, const char *sensorName, const char *unit, uint32_t value) {

    otic_ts_handler(channel, timestamp);
    otic_entry_t *entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        otic_str_t s = {sensorName, sensorName? strlen(sensorName) : 0}, u = {unit, unit? strlen(unit): 0};
        if (s.size + u.size > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_un(channel, &s, &u, channel->totalEntries, value)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_INT32_NEG);
        write_long(channel, channel->totalEntries);
        write_long(channel, value);
        channel->totalEntries++;
    } else{
        if (entry->type == OTIC_TYPE_INT32_NEG && entry->last_value.int_value == value){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            entry->last_value.int_value = value;
            entry->type = OTIC_TYPE_INT32_NEG;
            otic_pack_write_byte(channel, OTIC_TYPE_INT32_NEG);
            write_long(channel, entry->index);
            write_long(channel, value);
        }
    }
    channel->base.rowCounter++;
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_CLOSED);
    return 0;
}

uint8_t otic_pack_channel_inject_i(otic_pack_channel_t* channel, double timestamp, const char *sensorName, const char *sensorUnit, uint32_t value) {
    otic_ts_handler(channel, timestamp);
    otic_entry_t *entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        otic_str_t s = {sensorName, sensorName? strlen(sensorName) : 0}, u = {sensorUnit, sensorUnit? strlen(sensorUnit): 0};
        if (s.size + u.size > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_u(channel, &s, &u, channel->totalEntries, value)) {
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_INT32_POS);
        write_long(channel, channel->totalEntries);
        write_long(channel, value);
        channel->totalEntries++;
    } else{
        if (entry->type == OTIC_TYPE_INT32_POS && entry->last_value.int_value == value){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            entry->last_value.int_value = value;
            entry->type = OTIC_TYPE_INT32_POS;
            otic_pack_write_byte(channel, OTIC_TYPE_INT32_POS);
            write_long(channel, entry->index);
            write_long(channel, value);
        }
    }
    channel->base.rowCounter++;
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_inject_d(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, double value)
{
    otic_ts_handler(channel, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry) {
        otic_str_t s = {sensorName, sensorName? strlen(sensorName) : 0}, u = {unit, unit? strlen(unit): 0};
        if (s.size + u.size > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_d(channel, &s, &u, channel->totalEntries, value);
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_DOUBLE);
        write_long(channel, channel->totalEntries);
        write_double(channel, value);
        channel->totalEntries++;
    } else {
        if (entry->type == OTIC_TYPE_DOUBLE && entry->last_value.double_value == value) {
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            entry->last_value.double_value = value;
            entry->type = OTIC_TYPE_DOUBLE;
            otic_pack_write_byte(channel, OTIC_TYPE_DOUBLE);
            write_long(channel, entry->index);
            write_double(channel, value);
        }
    }
    channel->base.rowCounter++;
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_inject_s(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, const char* value)
{
    otic_ts_handler(channel, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    otic_str_t v = {value, value ? strlen(value) : 0};
    if (v.size > UINT8_MAX){
        otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
        goto fail;
    }
    if (!entry){
        otic_str_t s = {sensorName, sensorName? strlen(sensorName) : 0}, u = {unit, unit? strlen(unit): 0};
        if (s.size + u.size > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_s(channel, &s, &u, channel->totalEntries, &v);
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_STRING);
        write_long(channel, channel->totalEntries);
        write_string(channel, &v);
        channel->totalEntries++;
    } else {
        if (entry->type == OTIC_TYPE_STRING && strncmp(entry->last_value.string_value.ptr, value, v.size) == 0){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            // TODO: Only reallocate, no allocation needed, and reallocate only if the size is bigger than the old size
            if (entry->last_value.string_value.size == 0)
                entry->last_value.string_value.ptr = malloc(v.size);
            else
                entry->last_value.string_value.ptr = realloc((char*)entry->last_value.string_value.ptr, v.size);
            memcpy((char*)entry->last_value.string_value.ptr, value, v.size);
            otic_pack_write_byte(channel, OTIC_TYPE_STRING);
            write_long(channel, entry->index);
            write_string(channel, &v);
        }
    }
    channel->base.rowCounter++;
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_inject_n(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit)
{
    otic_ts_handler(channel, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry) {
        otic_str_t s = {sensorName, sensorName? strlen(sensorName) : 0}, u = {unit, unit? strlen(unit): 0};
        if (s.size + u.size > UINT8_MAX) {
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_n(channel, &s, &u, channel->totalEntries)) {
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_entry_insert_n(channel, &s, &u, channel->totalEntries);
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_NULL);
        write_long(channel, channel->totalEntries);
        channel->totalEntries++;
    } else{
        if (entry->type == OTIC_TYPE_NULL) {
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            entry->type = OTIC_TYPE_NULL;
            otic_pack_write_byte(channel, OTIC_TYPE_NULL);
            write_long(channel, entry->index);
        }
    }
    channel->base.rowCounter++;
    otic_pack_flush_if_flushable(channel);
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_CLOSED);
    return 0;
}

uint8_t otic_pack_channel_inject_b(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, uint8_t* buffer, size_t size)
{
    otic_ts_handler(channel, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        otic_str_t s = {sensorName, sensorName? strlen(sensorName) : 0}, u = {unit, unit? strlen(unit): 0};
        if (s.size + u.size > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_b(channel, &s, &u, channel->totalEntries)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_RAWBUFFER);
        write_long(channel, channel->totalEntries);
        channel->totalEntries++;
    } else {
        entry->type = OTIC_TYPE_RAWBUFFER;
        otic_pack_write_byte(channel, OTIC_TYPE_RAWBUFFER);
        write_long(channel, entry->index);
    }
    write_buffer(channel, buffer, size);
    channel->base.rowCounter++;
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_CLOSED);
    return 0;
}

uint8_t otic_pack_channel_init(otic_pack_channel_t* channel, uint8_t id, channel_type_e channelType, const char* metaData, otic_pack_t* parent)
{
    if (!channel)
        return 0;
    if (!otic_base_init(&channel->base))
        return 0;
    channel->cCtx = ZSTD_createCCtx();
    if (!channel->cCtx){
        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
    size_t counter;
    for (counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
        channel->cache[counter] = 0;
    channel->totalEntries = 0;
    channel->threshold = &channel->base.cache[OTIC_BASE_CACHE_SIZE - OTIC_PACK_CACHE_TOP_LIMIT];
    channel->info.channelId = id;
    channel->info.channelType = channelType;
    channel->info.parent = parent;
    return 1;
fail:
    return 0;
}

uint8_t otic_pack_channel_close(otic_pack_channel_t* channel)
{
    if (!channel)
        return 0;
    otic_pack_write_byte(channel, OTIC_TYPE_EOF);
    write_long(channel, channel->base.rowCounter);
    otic_pack_channel_flush(channel);
    ZSTD_freeCCtx(channel->cCtx);
    otic_entry_t* temp = 0;
    otic_entry_t* next = 0;

    for (size_t counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
    {
        next = channel->cache[counter];
        while (next)
        {
            free(next->name);
            if (next->last_value.string_value.size != 0)
                free((char *)next->last_value.string_value.ptr);
            temp = next->next;
            free(next);
            next = temp;
        }
    }
    channel->info.parent->totalChannels--;
    free(channel);
    return 1;
}

uint8_t otic_pack_channel_flush(otic_pack_channel_t* channel)
{
#if OTIC_PACK_NO_COMPRESSION
    otic_payload_t payload = {.dataLen = channel->base.top - channel->base.cache, .startTimestamp = channel->base.timestamp_start, .channelId = channel->info.channelId};
    if (!channel->info.parent->flusher((uint8_t*)&payload, sizeof(payload))){
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
    if (!channel->info.parent->flusher(channel->base.cache, channel->base.top - channel->base.cache)){
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
#else
    size_t ret = ZSTD_compressCCtx(
            channel->cCtx,
            channel->ztd_out,
            ZSTD_OUT_SIZE,
            channel->base.cache,
            channel->base.top - channel->base.cache,
            OTIC_ZSTD_COMPRESSION_LEVEL
            );
    if (ZSTD_isError(ret)) {
        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
    otic_payload_t payload = {.startTimestamp = channel->base.timestamp_start, .dataLen = ret, .channelId = channel->info.channelId};
    uint8_t val = OTIC_TYPE_DATA;
    channel->info.parent->flusher(&val, 1, channel->info.parent->data);
    if (!channel->info.parent->flusher((uint8_t*)&payload, sizeof(otic_payload_t), channel->info.parent->data)) {
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
    if (!channel->info.parent->flusher(channel->ztd_out, ret, channel->info.parent->data)) {
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
#endif
    channel->base.timestamp_start = 0;
    channel->base.top = channel->base.cache;
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    otic_pack_setState(channel->info.parent, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_init(otic_pack_t* oticPack, uint8_t(*flusher)(uint8_t*, size_t, void*), void* data)
{
    if (!oticPack)
        return 0;
    if (!flusher) {
        otic_pack_setError(oticPack, OTIC_ERROR_INVALID_POINTER);
        goto fail;
    }
    oticPack->flusher = flusher;
    oticPack->data = data;
    oticPack->channels = 0;
    oticPack->totalChannels = 0;
    flusher((uint8_t*)&(otic_header_t){.magic = "OC\x07\xFF", .features=0x00, .version = OTIC_VERSION_MAJOR}, sizeof(otic_header_t), data);
    otic_pack_setState(oticPack, OTIC_STATE_OPENED);
    otic_pack_setError(oticPack, OTIC_ERROR_NONE);
    return 1;
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

/**
 * @param oticPack
 * @param channelType
 * @param id
 * @param features
 * @warning The result of this function needs to be used! \a __attribute__((warn_unused_result)) is specified to warn the
 * developer if this shouldn't be the case
 * @attention 0 is the reserved ID for end of file.
 * @return A Pointer to the new channel in case of successful allocation, else a nullptr in case of failure
 * Reasons for failure could be an invalid or already occupied id, allocation/reallocation failure or an invalid
 * pointer to \a oticPack
 */
otic_pack_channel_t* otic_pack_defineChannel(otic_pack_t* oticPack, channel_type_e channelType, uint8_t id, otic_features_e features)
{
    if (!oticPack)
        return 0;
    if (otic_pack_getState(oticPack) != OTIC_STATE_OPENED) {
        otic_pack_setError(oticPack, OTIC_ERROR_AT_INVALID_STATE);
        goto fail;
    }
    uint8_t counter;
    for (counter = 0; counter < oticPack->totalChannels; counter++)
        if (oticPack->channels[counter]->info.channelId == id) {
            otic_pack_setError(oticPack, OTIC_ERROR_INVALID_ARGUMENT);
            goto fail;
        }
    otic_pack_channel_t** ptr = realloc(oticPack->channels, sizeof(otic_pack_channel_t*) * (oticPack->totalChannels + 1));
    if (!ptr) {
        otic_pack_setError(oticPack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    oticPack->channels = ptr;
    oticPack->channels[oticPack->totalChannels] = calloc(1, sizeof(otic_pack_channel_t));
//    oticPack->channels[oticPack->totalChannels] = malloc(sizeof(otic_pack_channel_t));
    if (!oticPack->channels[oticPack->totalChannels]){
        otic_pack_setError(oticPack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    otic_meta_data_t oticMetaData[3] = {
            {.metaType = OTIC_META_TYPE_CHANNEL_DEFINE, .metaArg = id},
            {.metaType = OTIC_META_TYPE_CHANNEL_TYPE, .metaArg = channelType},
            {.metaType = OTIC_META_TYPE_COMPRESSION_METHOD, .metaArg = OTIC_FEATURE_COMPRESSION_ZSTD}
    };
    otic_meta_head_t oticMetaHead = {.otic_type = OTIC_TYPE_METADATA, .metaDataSize = sizeof(oticMetaData)/ sizeof(otic_meta_data_t)};
    oticPack->flusher((uint8_t*)&oticMetaHead, sizeof(otic_meta_head_t), oticPack->data);
    oticPack->flusher((uint8_t*)&oticMetaData, sizeof(oticMetaData), oticPack->data);
    otic_pack_channel_init(oticPack->channels[oticPack->totalChannels], id, channelType, "", oticPack);
    return oticPack->channels[oticPack->totalChannels++];
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_closeChannel(otic_pack_t* oticPack, uint8_t id)
{
    uint8_t counter;
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        if (oticPack->channels[counter]->info.channelId == id)
        {
            otic_pack_channel_close(oticPack->channels[counter]);
            otic_pack_channel_t* temp = oticPack->channels[counter];
            oticPack->channels[counter] = oticPack->channels[oticPack->totalChannels];
            free(temp);
            oticPack = realloc(oticPack, sizeof(otic_pack_channel_t*) * oticPack->totalChannels);
            return 1;
        }
    }
    otic_pack_setError(oticPack, OTIC_ERROR_INVALID_ARGUMENT);
    return 0;
}

uint8_t otic_pack_flush(otic_pack_t* oticPack)
{
    uint8_t counter;
    otic_payload_t payload;
#if OTIC_PACK_NO_COMPRESSION
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        payload.channelId = oticPack->channels[counter]->info.channelId;
        payload.dataLen = oticPack->channels[counter]->base.top - oticPack->channels[counter]->base.cache;
        payload.startTimestamp = oticPack->channels[counter]->base.timestamp_current;
        if (!oticPack->flusher((uint8_t*)&payload, sizeof(otic_payload_t))){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        if (!oticPack->flusher(oticPack->channels[counter]->base.cache, oticPack->channels[counter]->base.top - oticPack->channels[counter]->base.cache)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        oticPack->channels[counter]->base.top = oticPack->channels[counter]->base.cache;
    }
#else
    size_t ret;
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        ret = ZSTD_compressCCtx(
                oticPack->channels[counter]->cCtx, oticPack->channels[counter]->ztd_out, ZSTD_OUT_SIZE,
                oticPack->channels[counter]->base.cache,
                oticPack->channels[counter]->base.top - oticPack->channels[counter]->base.cache, OTIC_ZSTD_COMPRESSION_LEVEL
                );
        if (ZSTD_isError(ret)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_ZSTD);
            goto fail;
        }
        payload.channelId = oticPack->channels[counter]->info.channelId;
        payload.dataLen = ret;
        payload.startTimestamp = oticPack->channels[counter]->base.timestamp_current;
        if (!oticPack->flusher((uint8_t*)&payload, sizeof(otic_payload_t), oticPack->data)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        if (!oticPack->flusher(oticPack->channels[counter]->ztd_out, ret, oticPack->data)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        oticPack->channels[counter]->base.top = oticPack->channels[counter]->base.cache;
    }
#endif
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

void otic_pack_close(otic_pack_t* oticPack) {
    if (!oticPack)
        return;
    uint8_t counter = oticPack->totalChannels;
    while (counter--)
        otic_pack_channel_close(oticPack->channels[counter]);
    free(oticPack->channels);
    otic_pack_setState(oticPack, OTIC_STATE_CLOSED);
}
