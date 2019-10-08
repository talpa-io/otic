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
static otic_entry_t* otic_pack_entry_find(otic_pack_channel_t* pack, const char* o)
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
static otic_entry_t* otic_pack_entry_insert_routine(otic_pack_channel_t* channel, const char* o, const char* unit, uint16_t index)
{
    uint32_t hash_address = otic_hashFunction(o);
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
    ptr->last_value.string_value.value = 0;
    ptr->last_value.string_value.size = 0;
    uint8_t len[2] = {strlen(o), strlen(unit)};
    ptr->name = malloc(len[0] + len[1] + 1);
    strcpy(ptr->name, o);
    ptr->name[len[0] + 1] = ':';
    strcpy(ptr->name + len[0] + 2, unit);
    ptr->index = index;
    ptr->next = channel->cache[hash_address];
    channel->cache[hash_address] = ptr;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_u(otic_pack_channel_t* channel, const char* o, const char* unit, uint16_t index, uint32_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_INT32_POS;
    ptr->last_value.int_value = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_un(otic_pack_channel_t* channel, const char* o, const char* unit, uint16_t index, uint32_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_INT32_NEG;
    ptr->last_value.int_value = value;
    return ptr;
}


OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_d(otic_pack_channel_t* channel, const char* o, const char* unit, uint16_t index, double value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_DOUBLE;
    ptr->last_value.double_value = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_s(otic_pack_channel_t* channel, const char* o, const char* unit, uint16_t index, const char* value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_STRING;
    ptr->last_value.string_value.size = strlen(value);
    ptr->last_value.string_value.value = malloc(ptr->last_value.string_value.size + 1);
    strcpy(ptr->last_value.string_value.value, value);
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_n(otic_pack_channel_t* channel, const char* o, const char* unit, uint16_t index){
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->type = OTIC_TYPE_NULL;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_b(otic_pack_channel_t* channel, const char* o, const char* unit, uint16_t index)
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
void static write_long(otic_pack_channel_t* channel, long value)
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
static void write_string(otic_pack_channel_t* channel, const char* value, uint8_t len)
{
    otic_pack_write_byte(channel, len);
    memcpy(channel->base.top, value, len);
    channel->base.top += len;
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
void static otic_pack_id_assign(otic_pack_channel_t* channel, const char* sensorName, const char* unit)
{
    otic_pack_write_byte(channel, OTIC_TYPE_NAME_ASSIGN);
    uint8_t len[2] = {strlen(sensorName), strlen(unit)};
    otic_pack_write_byte(channel, len[0] + len[1] + 1);
    memcpy(channel->base.top, sensorName, len[0]);
    channel->base.top += len[0];
    otic_pack_write_byte(channel, ':');
    memcpy(channel->base.top, unit, len[1]);
    channel->base.top += len[1];
}

OTIC_PACK_INLINE
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
}

OTIC_PACK_INLINE
void static otic_metadata_write(otic_pack_channel_t* channel, const char* value)
{
    otic_pack_write_byte(channel, OTIC_TYPE_METADATA);
    write_string(channel, value, strlen(value));
}

OTIC_PACK_INLINE
static uint8_t otic_pack_flush(otic_pack_channel_t* channel)
{
#if OTIC_PACK_NO_COMPRESSION
    if (!channel->flusher[0](channel->base.cache, channel->base.top - channel->base.cache)){
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
#else
    size_t ret = ZSTD_compressCCtx(channel->cCtx, channel->ztd_out, ZSTD_OUT_SIZE, channel->base.cache, channel->base.top - channel->base.cache, 1);
    if (ZSTD_isError(ret)){
        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
//    if (!channel->flusher((uint8_t*)&ret, sizeof(uint32_t))){
//        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
//        goto fail;
//    }
//    if (!channel->flusher(channel->zstd_out, ret)){
//        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
//        goto fail;
//    }
#endif
    channel->base.top = channel->base.cache;
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

OTIC_PACK_INLINE
void static otic_pack_flush_if_flushable(otic_pack_channel_t* channel)
{
    if (channel->base.top > channel->threshold)
        otic_pack_flush(channel);
}

OTIC_PACK_INLINE
static uint8_t otic_pack_init(otic_pack_channel_t* channel)
{
    if (!otic_base_init(&channel->base))
        goto fail;
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
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_inject_i_neg(otic_pack_channel_t* channel, double timestamp, const char *sensorName, const char *unit, uint32_t value) {
    otic_ts_handler(channel, timestamp);
    otic_entry_t *entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_un(channel, sensorName, unit, channel->totalEntries, value)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, sensorName, unit);
        otic_pack_write_byte(channel, OTIC_TYPE_INT32_NEG);
        write_long(channel, channel->totalEntries);
        write_long(channel, value);
        channel->totalEntries++;
    } else{
        if (entry->last_value.int_value == value && entry->type == OTIC_TYPE_INT32_NEG){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            entry->last_value.int_value = value;
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
        size_t len[2] = {strlen(sensorName), strlen(sensorUnit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_u(channel, sensorName, sensorUnit, channel->totalEntries, value)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, sensorName, sensorUnit);
        otic_pack_write_byte(channel, OTIC_TYPE_INT32_POS);
        write_long(channel, channel->totalEntries);
        write_long(channel, value);
        channel->totalEntries++;
    } else{
        if (entry->last_value.int_value == value){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            entry->last_value.int_value = value;
            otic_pack_write_byte(channel, OTIC_TYPE_INT32_POS);
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

uint8_t otic_pack_channel_inject_d(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, double value)
{
    otic_ts_handler(channel, timestamp);
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_d(channel, sensorName, unit, channel->totalEntries, value);
        otic_pack_id_assign(channel, sensorName, unit);
        otic_pack_write_byte(channel, OTIC_TYPE_DOUBLE);
        write_long(channel, channel->totalEntries);
        write_double(channel, value);
        channel->totalEntries++;
    } else {
        if (entry->last_value.double_value == value) {
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            entry->last_value.double_value = value;
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
    size_t len1 = strlen(value);
    if (len1 > UINT8_MAX){
        otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
        goto fail;
    }
    if (!entry){
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_s(channel, sensorName, unit, channel->totalEntries, value);
        otic_pack_id_assign(channel, sensorName, unit);
        otic_pack_write_byte(channel, OTIC_TYPE_STRING);
        write_long(channel, channel->totalEntries);
        write_string(channel, value, len1);
        channel->totalEntries++;
    } else {
        if (strcmp(entry->last_value.string_value.value, value) == 0){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            if (entry->last_value.string_value.size < strlen(value))
                entry->last_value.string_value.value = realloc(entry->last_value.string_value.value, len1 + 1);
            strcpy(entry->last_value.string_value.value, value);
            otic_pack_write_byte(channel, OTIC_TYPE_STRING);
            write_long(channel, entry->index);
            write_string(channel, value, len1);
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
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_n(channel, sensorName, unit, channel->totalEntries)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, sensorName, unit);
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
        size_t len[2] = {strlen(sensorName), strlen(unit)};
        if (len[0] + len[1] > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_b(channel, sensorName, unit, channel->totalEntries)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, sensorName, unit);
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

OTIC_PACK_INLINE
static uint8_t otic_pack_close(otic_pack_channel_t* channel)
{
    if (!channel)
        goto fail;
    otic_pack_write_byte(channel, OTIC_TYPE_EOF);
    write_long(channel, channel->base.rowCounter);
    otic_pack_flush(channel);
    ZSTD_freeCCtx(channel->cCtx);
    size_t counter;
    for (counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
    {
        if (channel->cache[counter] != 0)
        {
            free(channel->cache[counter]->name);
            if (channel->cache[counter]->last_value.string_value.size)
                free(channel->cache[counter]->last_value.string_value.value);
            // TODO: Free
//            free(channel->cache[counter]);
//            otic_entry_t* ptr = channel->cache[counter];
//            otic_entry_t* next = 0;
//            while(ptr != 0)
//            {
//                next = ptr->next;
//                free(ptr);
//                ptr = next;
//            }
//            free(channel->cache[counter]);
        }
    }
    return 1;
fail:
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
    channel->threshold = &channel->base.cache[OTIC_PACK_CACHE_SIZE - OTIC_PACK_CACHE_TOP_LIMIT];
    channel->info_t.channelId = id;
    channel->info_t.channelType = channelType;
    channel->info_t.parent = parent;
    return 1;
fail:
    return 0;
}

uint8_t otic_pack_channel_close(otic_pack_channel_t* channel)
{
    if (!channel)
        goto fail;
    // TODO: IMPLEMENT CLOSE
    return 1;
fail:
    return 0;
}

uint8_t otic_pack_base_init(otic_pack_t* oticPack, uint8_t(*flusher)(uint8_t*, size_t))
{
    if (!oticPack)
        return 0;
    if (!flusher)
        return 0;
    oticPack->flusher = flusher;
    oticPack->channels = 0;
    oticPack->totalChannels = 0;
    otic_header_t header = {.magic = "OC\x07\xFF", .features = 0x00, .version = OTIC_VERSION_MAJOR};
    flusher((uint8_t*)&header, sizeof(otic_header_t));
    return 1;
}

/**
 * @param oticPack
 * @param channelType
 * @param id
 * @param features
 * @warning The result of this function needs to be used. \a __attribute__((warn_unused_result)) is specified to warn the
 * developer if this shouldn't be the case
 * @attention 0 is the reserved ID for end of file.
 * @return A Pointer to the new channel in case of successful allocation, else a nullptr in case of failure
 * Reasons for failure could be an invalid or already occupied id, allocation/reallocation failure or an invalid
 * pointer to \a oticPack
 */
otic_pack_channel_t* otic_pack_base_defineChannel(otic_pack_t* oticPack, channel_type_e channelType, uint8_t id, otic_features_e features)
{
    if (!oticPack || id == 0)
        return 0;
    uint8_t counter;
    for (counter = 0; counter < oticPack->totalChannels; counter++)
        if (oticPack->channels[counter]->info_t.channelId == id)
            return 0;
    oticPack->channels = realloc(oticPack->channels, sizeof(otic_pack_channel_t*) * (oticPack->totalChannels + 1));
    if (!oticPack->channels)
        return 0;
    oticPack->channels[oticPack->totalChannels] = malloc(sizeof(otic_pack_channel_t));
    if (!oticPack->channels[oticPack->totalChannels])
        return 0;
    otic_meta_channel_t oticMetaChannel[3] = {
            {.metaType = OTIC_META_TYPE_CHANNEL_DEFINE, .channelId = id, .opt_value = 0x00},
            {.metaType = OTIC_META_TYPE_CHANNEL_TYPE, .channelId = id, .opt_value = channelType},
            {.metaType = OTIC_META_TYPE_COMPRESSION_METHOD, .channelId = id, .opt_value = OTIC_FEATURE_COMPRESSION_ZSTD}
    };
    oticPack->flusher((uint8_t*)&oticMetaChannel, sizeof(oticMetaChannel));
    otic_pack_channel_init(oticPack->channels[oticPack->totalChannels], id, channelType, "", oticPack);
    return oticPack->channels[oticPack->totalChannels++];
}

uint8_t otic_pack_base_closeChannel(otic_pack_t* oticPack, uint8_t id)
{
    uint8_t counter;
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        if (oticPack->channels[counter]->info_t.channelId == id)
        {
            otic_pack_channel_close(oticPack->channels[counter]);
            otic_pack_channel_t* temp = oticPack->channels[counter];
            oticPack->channels[counter] = oticPack->channels[oticPack->totalChannels];
            free(temp);
            oticPack = realloc(oticPack, sizeof(otic_pack_channel_t*) * --oticPack->totalChannels);
            return 1;
        }
    }
    return 0;
}

uint8_t otic_pack_base_flush(otic_pack_t* oticPack)
{
    uint8_t counter;
#if OTIC_PACK_NO_COMPRESSION
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        if (!oticPack->flusher(oticPack->channels[counter]->base.cache, oticPack->channels[counter]->base.top - oticPack->channels[counter]->base.cache)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        oticPack->channels[counter]->base.top = oticPack->channels[counter]->base.cache;
    }
#else
    size_t ret;
    otic_payload_t payload;
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        ret = ZSTD_compressCCtx(oticPack->channels[counter]->cCtx, oticPack->channels[counter]->ztd_out, ZSTD_OUT_SIZE, oticPack->channels[counter]->base.cache, oticPack->channels[counter]->base.top - oticPack->channels[counter]->base.cache, 1);
        if (ZSTD_isError(ret)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        payload.channelId = oticPack->channels[counter]->info_t.channelId;
        payload.dataLen = ret;
        payload.startTimestamp = oticPack->channels[counter]->base.timestamp_current;
        if (!oticPack->flusher((uint8_t*)&payload, sizeof(otic_payload_t))){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        if (!oticPack->flusher(oticPack->channels[counter]->ztd_out, ret)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        oticPack->channels[counter]->base.top = oticPack->channels[counter]->base.cache;
    }
#endif
fail:
    return 0;
}

void otic_pack_base_close(otic_pack_t* oticPack)
{
    if (!oticPack)
        return;
    uint8_t counter = 0;
    otic_pack_base_flush(oticPack);
    for (counter = 0; counter < oticPack->totalChannels; counter++)
        free(oticPack->channels[counter]);
    free(oticPack->channels);
}