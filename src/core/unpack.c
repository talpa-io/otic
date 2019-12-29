#include <fenv.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <zstd.h>
#include "core/unpack.h"
#include "core/config.h"


#if OTIC_UNPACK_INLINE_ALL_STATIC
#define OTIC_UNPACK_INLINE inline
#else
#define OTIC_UNPACK_INLINE
#endif


OTIC_UNPACK_INLINE
__attribute_pure__ static size_t otic_hashFunction(const char* ptr)
{
    size_t hash_address = 0;
    while(*ptr)
        hash_address = PTR_M * hash_address + *ptr++;
    return hash_address;
}

OTIC_UNPACK_INLINE
static uint8_t isFetchable(oticUnpackChannel_t* channel, const char* value)
{
    size_t hashAddress = otic_hashFunction(value);
    for (size_t counter = 0; counter < channel->toFetch.size; counter++)
        if (hashAddress == channel->toFetch.ptr[counter])
            return 1;
    return 0;
}

OTIC_UNPACK_INLINE
static oticUnpackEntry_t* otic_unpack_insert_entry(oticUnpackChannel_t* channel, char* value, const char* end) {
    if (channel->cache.allocationLeft == 0) {
        oticUnpackEntry_t** temp = realloc(channel->cache.cache, sizeof(*channel->cache.cache) * (OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE + channel->cache.cache_allocated));
        if (!temp)
            return 0;
        channel->cache.cache = temp;
        for (size_t counter = channel->cache.cache_allocated; counter <  channel->cache.cache_allocated + OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE; counter++)
            channel->cache.cache[counter] = 0;
        channel->cache.allocationLeft = OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE;
        channel->cache.cache_allocated += OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE;
    }
    char *ptr = value;
    while (ptr <= end)
    {
        if (*ptr == ':') {
            *ptr = 0;
            ptr++;
            break;
        }
        ptr++;
    }
    channel->cache.cache[channel->cache.totalEntries] = malloc(sizeof(oticUnpackEntry_t));
    oticUnpackEntry_t* entry = channel->cache.cache[channel->cache.totalEntries];
    uint8_t lengthValue = (ptr - value);
    entry->index = channel->cache.totalEntries;
    entry->name = malloc((lengthValue + 1) * sizeof(char));
    memcpy(entry->name, value, lengthValue);
    entry->ignore = 0;
    entry->name[lengthValue] = 0;
    entry->unit = malloc((end - ptr + 1u) * sizeof(char));
    entry->unit[end - ptr] = 0;
    memcpy(entry->unit, ptr,  end - ptr);
    entry->value.sval.size = 0;
    entry->value.sval.ptr = 0;
    entry->ignore = !isFetchable(channel, entry->name);
    --channel->cache.allocationLeft;
    return channel->cache.cache[channel->cache.totalEntries];
}

OTIC_UNPACK_INLINE
static void flush_if_flushable(oticUnpackChannel_t* channel)
{
    if (channel->cache.currentEntry->ignore && channel->toFetch.size != 0)
        return;
    channel->flusher(channel->ts, channel->cache.currentEntry->name, channel->cache.currentEntry->unit, &channel->cache.currentEntry->value, channel->data);
}


OTIC_UNPACK_INLINE
static void otic_unpack_read_null(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    channel->cache.currentEntry->value.type = OTIC_TYPE_NULL;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_empty(oticUnpackChannel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_int32neg(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    if (channel->cache.currentEntry->value.type == OTIC_TYPE_STRING) {
        free((void*)channel->cache.currentEntry->value.sval.ptr);
        channel->cache.currentEntry->value.sval.ptr = 0;
        channel->cache.currentEntry->value.sval.size = 0;
    }
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->cache.currentEntry->value.lval);
    channel->cache.currentEntry->value.type = OTIC_TYPE_INT_NEG;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_int32pos(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    if (channel->cache.currentEntry->value.type == OTIC_TYPE_STRING) {
        free((void*)channel->cache.currentEntry->value.sval.ptr);
        channel->cache.currentEntry->value.sval.ptr = 0;
        channel->cache.currentEntry->value.sval.size = 0;
    }
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->cache.currentEntry->value.lval);
    channel->cache.currentEntry->value.type = OTIC_TYPE_INT_POS;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_double(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    if (channel->cache.currentEntry->value.type == OTIC_TYPE_STRING) {
        free((void*)channel->cache.currentEntry->value.sval.ptr);
        channel->cache.currentEntry->value.sval.ptr = 0;
        channel->cache.currentEntry->value.sval.size = 0;
    }
    memcpy(&channel->cache.currentEntry->value.dval, channel->base.top, sizeof(double));
    channel->base.top += sizeof(double);
    channel->cache.currentEntry->value.type = OTIC_TYPE_DOUBLE;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min1float(oticUnpackChannel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min2float(oticUnpackChannel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min3float(oticUnpackChannel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_float(oticUnpackChannel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_meddouble(oticUnpackChannel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_string(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    uint8_t total = *channel->base.top++;
    if (channel->cache.currentEntry->value.type != OTIC_TYPE_STRING)
        channel->cache.currentEntry->value.sval.ptr = 0;
    if (channel->cache.currentEntry->value.sval.size < total) {
        channel->cache.currentEntry->value.sval.ptr = realloc((char *) channel->cache.currentEntry->value.sval.ptr, (total + 1) * sizeof(char));
        channel->cache.currentEntry->value.sval.size = total;
    }
    memcpy((void*)channel->cache.currentEntry->value.sval.ptr, channel->base.top, total);
    channel->cache.currentEntry->value.sval.ptr[total] = 0;
    channel->cache.currentEntry->value.type = OTIC_TYPE_STRING;
    channel->base.top += total;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_unmodified(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_rawbuffer(oticUnpackChannel_t* channel)
{
    uint32_t size;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &size);
    channel->base.top += size;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_setTimestamp(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint32_t*)&channel->base.timestamp_start);
    channel->base.timestamp_current = channel->base.timestamp_start;
    channel->ts = (double)channel->base.timestamp_current / OTIC_TS_MULTIPLICATOR;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_shiftTimestamp(oticUnpackChannel_t* channel)
{
    uint32_t value;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &value);
    channel->base.timestamp_current += value;
    channel->ts = (double)channel->base.timestamp_current / OTIC_TS_MULTIPLICATOR;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_fileVersion(oticUnpackChannel_t* channel)
{
    if (*channel->base.top++ < OTIC_VERSION_MAJOR) {
        otic_base_setError(&channel->base, OTIC_ERROR_VERSION_UNSUPPORTED);
        otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    }
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_nameAssign(oticUnpackChannel_t* channel)
{
    uint32_t length;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &length);
    channel->cache.currentEntry = otic_unpack_insert_entry(channel, (char*)channel->base.top, (const char*)channel->base.top + length);
    channel->base.top += length;
    channel->cache.currentEntry->value.type = OTIC_TYPE_EOF;
    ++channel->cache.totalEntries;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_eof(oticUnpackChannel_t* channel)
{
    uint32_t expected = 0;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &expected);
    if (expected != channel->base.rowCounter) {
        otic_base_setError(&channel->base, OTIC_ERROR_ROW_COUNT_MISMATCH);
        otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    }
}

OTIC_UNPACK_INLINE
static void otic_unpack_setError(otic_unpack_t* oticUnpack, otic_errors_e error)
{
    oticUnpack->error = error;
}

OTIC_UNPACK_INLINE
static otic_errors_e otic_unpack_getError(otic_unpack_t* oticUnpack)
{
    return oticUnpack->error;
}

OTIC_UNPACK_INLINE
static void otic_unpack_setState(otic_unpack_t* oticUnpack, otic_state_e state)
{
    oticUnpack->state = state;
}

OTIC_UNPACK_INLINE
static otic_state_e otic_unpack_getState(otic_unpack_t* oticUnpack)
{
    return oticUnpack->state;
}

uint8_t otic_unpack_channel_init(oticUnpackChannel_t* channel, uint8_t id, uint8_t(*flusher)(double, const char*, const char*, const oval_t*, void*),void* data, otic_unpack_t* parent)
{
    channel->data = data;
    channel->flusher = flusher;
    if (!(channel->dCtx = ZSTD_createDCtx())){
        otic_unpack_setError(parent, OTIC_ERROR_ZSTD);
        goto fail;
    }
    otic_base_init(&channel->base);
    channel->info.channelId = id;
    channel->info.metaData = 0;
    channel->info.parent = parent;
    channel->blockSize = 0;
    channel->cache.currentEntry = 0;
    channel->cache.cache = 0;
    channel->toFetch.ptr = 0;
    channel->toFetch.size = 0;
    channel->cache.cache_allocated = channel->cache.totalEntries = channel->cache.allocationLeft = 0;
    return 1;
fail:
    otic_unpack_setState(channel->info.parent, OTIC_STATE_ON_ERROR);
    return 0;
}

void otic_unpack_channel_toFetch(oticUnpackChannel_t* channel, const char** values, size_t size)
{
    channel->toFetch.ptr = malloc(sizeof(size_t) * size);
    channel->toFetch.size = size;
    for (size_t counter = 0; counter < size; ++counter)
        channel->toFetch.ptr[counter] = otic_hashFunction(values[counter]);
}

OTIC_UNPACK_INLINE
static uint8_t switchPopChannel(otic_unpack_t* oticUnpack, uint8_t id)
{
    for (uint8_t counter = 0; counter < oticUnpack->totalChannels; counter++)
    {
        if (oticUnpack->channels[counter]->info.channelId == id) {
            oticUnpackChannel_t* temp = oticUnpack->channels[counter];
            oticUnpack->channels[counter] = oticUnpack->channels[oticUnpack->totalChannels - 1];
            oticUnpack->channels[--oticUnpack->totalChannels] = temp;
            free(temp);
            return 1;
        }
    }
    return 0;
}

uint8_t otic_unpack_channel_close(oticUnpackChannel_t* channel)
{
    ZSTD_freeDCtx(channel->dCtx);
    size_t counter;
    for (counter = 0; counter < channel->cache.cache_allocated; ++counter)
    {
        if (channel->cache.cache[counter]) {
            if (channel->cache.cache[counter]->value.sval.size != 0)
                free((void*)channel->cache.cache[counter]->value.sval.ptr);
            free(channel->cache.cache[counter]->name);
            free(channel->cache.cache[counter]->unit);
            free(channel->cache.cache[counter]);
        }
    }
    free(channel->cache.cache);
    free(channel->toFetch.ptr);
    free(channel->info.metaData);
    return switchPopChannel(channel->info.parent, channel->info.channelId);
}

OTIC_UNPACK_INLINE
__attribute__((pure)) __attribute__((warn_unused_result)) static int32_t otic_unpack_getMeta(otic_meta_data_t* metaData, uint8_t size, uint32_t value)
{
    uint8_t counter;
    for (counter = 0; counter < size; counter++)
        if (metaData[counter].metaType == value)
            return metaData[counter].metaArg;
    return -1;
}

OTIC_UNPACK_INLINE
static uint8_t otic_unpack_readMetadata(otic_unpack_t* oticUnpack)
{
    uint8_t size = 0;
    oticUnpack->fetcher(&size, sizeof(uint8_t), oticUnpack->fetcherData);
    otic_meta_data_t oticMetaData[size];
    oticUnpack->fetcher((uint8_t*)&oticMetaData, sizeof(oticMetaData), oticUnpack->fetcherData);
    int32_t arg;
    if ((arg = otic_unpack_getMeta(oticMetaData, size, OTIC_META_TYPE_CHANNEL_DEFINE)) == -1)
        return 0;
    uint8_t counter;
    for (counter = 0; counter < oticUnpack->totalChannels; counter++)
        if (oticUnpack->channels[counter]->info.channelId == arg) {
            oticUnpack->channels[counter]->info.metaData = malloc(size * sizeof(otic_meta_data_t));
            return 1;
        }
    return 0;
}

//OTIC_UNPACK_INLINE
//static void otic_unpack_parseBlock(oticUnpackChannel_t* channel)
//{
//    while (channel->base.top - channel->out < channel->blockSize)
//        parsers[*channel->base.top++].parserFunc(channel);
//}

OTIC_UNPACK_INLINE
static void otic_unpack_parseBlock(oticUnpackChannel_t* channel)
{
    while (channel->base.top - channel->out < channel->blockSize)
    {
        switch (*channel->base.top++)
        {
            case OTIC_TYPE_UNMODIFIED:
                otic_unpack_read_unmodified(channel);
                break;
            case OTIC_TYPE_INT_POS:
                otic_unpack_read_int32pos(channel);
                break;
            case OTIC_TYPE_INT_NEG:
                otic_unpack_read_int32neg(channel);
                break;
            case OTIC_TYPE_DOUBLE:
                otic_unpack_read_double(channel);
                break;
            case OTIC_TYPE_FLOAT:
                otic_unpack_read_float(channel);
                break;
            case OTIC_TYPE_NULL:
                otic_unpack_read_null(channel);
                break;
            case OTIC_TYPE_SET_TIMESTAMP:
                otic_unpack_read_setTimestamp(channel);
                break;
            case OTIC_TYPE_SHIFT_TIMESTAMP:
                otic_unpack_read_shiftTimestamp(channel);
                break;
            case OTIC_TYPE_NAME_ASSIGN:
                otic_unpack_read_nameAssign(channel);
                break;
            case OTIC_TYPE_FILE_VERSION:
                otic_unpack_read_fileVersion(channel);
                break;
            case OTIC_TYPE_EOF:
                otic_unpack_read_eof(channel);
                break;
            case OTIC_TYPE_METADATA:
                otic_unpack_readMetadata(channel->info.parent);
                break;
            case OTIC_TYPE_STRING:
                otic_unpack_read_string(channel);
                break;
            case OTIC_TYPE_RAWBUFFER:
                otic_unpack_read_rawbuffer(channel);
                break;
        }
    }
}

OTIC_UNPACK_INLINE
static uint8_t otic_unpack_read_data(otic_unpack_t* oticUnpack)
{
    otic_payload_t payload;
    oticUnpack->fetcher((uint8_t*)&payload, sizeof(payload), oticUnpack->fetcherData);
    uint8_t counter;
    for (counter = 0; counter < oticUnpack->totalChannels; counter++)
    {
        if (oticUnpack->channels[counter]->info.channelId == payload.channelId) {
            oticUnpack->channels[counter]->ts  = (double)oticUnpack->channels[counter]->base.timestamp_start / OTIC_TS_MULTIPLICATOR;
            oticUnpack->channels[counter]->blockSize = payload.dataLen;
            oticUnpack->fetcher(oticUnpack->channels[counter]->base.cache, payload.dataLen, oticUnpack->fetcherData);
            oticUnpack->channels[counter]->blockSize = ZSTD_decompressDCtx(oticUnpack->channels[counter]->dCtx, oticUnpack->channels[counter]->out, OTIC_UNPACK_OUT_SIZE, oticUnpack->channels[counter]->base.cache, payload.dataLen);
            if (ZSTD_isError(oticUnpack->channels[counter]->blockSize)){
                otic_base_setError(&oticUnpack->channels[counter]->base, OTIC_ERROR_ZSTD);
                return 0;
            }
            oticUnpack->channels[counter]->base.top = oticUnpack->channels[counter]->out;
            otic_unpack_parseBlock(oticUnpack->channels[counter]);
            return 1;
        }
    }
    if (oticUnpack->seeker) {
        oticUnpack->seeker(payload.dataLen, oticUnpack->seekerData);
    } else {
        uint8_t ptr[payload.dataLen];
        oticUnpack->fetcher(ptr, payload.dataLen, oticUnpack->fetcherData);
    }
    return 1;
}

uint8_t otic_unpack_init(otic_unpack_t* oticUnpack, uint8_t(*fetcher)(uint8_t*, size_t, void*), void* fetcherData, uint8_t(*seeker)(uint32_t, void*), void* seekerData)
{
    oticUnpack->fetcher = fetcher;
    oticUnpack->seeker = seeker;
    oticUnpack->seekerData = seekerData;
    oticUnpack->fetcherData = fetcherData;
    otic_header_t header;
    fetcher((uint8_t*)&header, sizeof(header), fetcherData);
    if (strncmp((const char*)header.magic, "OC\x07\xFF", OTIC_MAGIC_SIZE) != 0) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_DATA_CORRUPTED);
        goto fail;
    }
    if (header.version > OTIC_VERSION_MAJOR) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_VERSION_UNSUPPORTED);
        goto fail;
    }
    oticUnpack->totalChannels = 0;
    oticUnpack->channels = 0;
    otic_unpack_setState(oticUnpack, OTIC_STATE_OPENED);
    otic_unpack_setError(oticUnpack, OTIC_ERROR_NONE);
    return 1;
fail:
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}

oticUnpackChannel_t* otic_unpack_defineChannel(otic_unpack_t* oticUnpack, uint8_t id, uint8_t(*flusher)(double, const char*, const char*, const oval_t*, void*), void* data)
{
    if (oticUnpack->state != OTIC_STATE_OPENED) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_AT_INVALID_STATE);
        goto fail;
    }
    oticUnpackChannel_t** ptr = realloc(oticUnpack->channels, sizeof(oticUnpackChannel_t*) * (oticUnpack->totalChannels + 1));
    if (!ptr){
        otic_unpack_setError(oticUnpack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    oticUnpack->channels = ptr;
    oticUnpack->channels[oticUnpack->totalChannels] = malloc(sizeof(oticUnpackChannel_t));
    if (!otic_unpack_channel_init(oticUnpack->channels[oticUnpack->totalChannels], id, flusher, data, oticUnpack))
        goto fail;
    return oticUnpack->channels[oticUnpack->totalChannels++];
fail:
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_unpack_parse(otic_unpack_t* oticUnpack)
{
    uint8_t value;
    if (oticUnpack->fetcher(&value, 1, oticUnpack->fetcherData) == 0)
        return 0;
    if (value == OTIC_TYPE_METADATA)
        return otic_unpack_readMetadata(oticUnpack);
    else if (value == OTIC_TYPE_DATA)
        return otic_unpack_read_data(oticUnpack);
    otic_unpack_setError(oticUnpack, OTIC_ERROR_DATA_CORRUPTED);
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_unpack_closeChannel(otic_unpack_t* oticUnpack, uint8_t id)
{
    uint8_t counter;
    for (counter = 0; counter < oticUnpack->totalChannels; counter++)
    {
        if (oticUnpack->channels[counter]->info.channelId == id) {
            otic_unpack_channel_close(oticUnpack->channels[counter]);
            return switchPopChannel(oticUnpack, id);
        }
    }
    return 0;
}

uint8_t otic_unpack_close(otic_unpack_t* oticUnpack)
{
    while (oticUnpack->totalChannels)
        if (!otic_unpack_channel_close(oticUnpack->channels[oticUnpack->totalChannels - 1]))
            goto fail;
    free(oticUnpack->channels);
    otic_unpack_setState(oticUnpack, OTIC_STATE_CLOSED);
    return 1;
fail:
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}
