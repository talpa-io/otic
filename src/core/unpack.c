#include <fenv.h>
#include <string.h>
#include <stdlib.h>
#include <zstd.h>
#include "core/unpack.h"
#include "core/config.h"
#include <core/base.h>
#include <otic.h>


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
        if (*ptr == 0) {
//            *ptr = 0;
            ++ptr;
            break;
        }
        ++ptr;
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
    entry->value.val.sval.size = 0;
    entry->value.val.sval.ptr = 0;
    entry->ignore = !isFetchable(channel, entry->name);
    --channel->cache.allocationLeft;
    return channel->cache.cache[channel->cache.totalEntries];
}

OTIC_UNPACK_INLINE
static void otic_unpack_cleaner(oval_t* oval)
{
    switch (oval->type)
    {
        case OTIC_TYPE_STRING:
            free(oval->val.sval.ptr);
            break;
        case OTIC_TYPE_ARRAY:
            free(oval->val.aval.elements);
            break;
        case OTIC_TYPE_OBJECT:
            free(oval->val.oval.elements);
            break;
    }
}

OTIC_UNPACK_INLINE
static void flush_if_flushable(const oticUnpackChannel_t* channel)
{
    if (channel->cache.currentEntry->ignore && channel->toFetch.size != 0)
        return;
    channel->flusher(channel->ts, channel->cache.currentEntry->name, channel->cache.currentEntry->unit, &channel->cache.currentEntry->value, channel->data);
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_true(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    channel->cache.currentEntry->value.type = OTIC_TYPE_TRUE;
    flush_if_flushable(channel);
    ++channel->base.rowCounter;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_false(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    channel->cache.currentEntry->value.type = OTIC_TYPE_FALSE;
    flush_if_flushable(channel);
    ++channel->base.rowCounter;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_null(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    otic_unpack_cleaner(&channel->cache.currentEntry->value);
    channel->cache.currentEntry->value.type = OTIC_TYPE_NULL;
    flush_if_flushable(channel);
    ++channel->base.rowCounter;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_empty(oticUnpackChannel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_smallint(oticUnpackChannel_t* channel, uint8_t val)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    otic_unpack_cleaner(&channel->cache.currentEntry->value);
    channel->cache.currentEntry->value.type = OTIC_TYPE_INT_POS;
    channel->cache.currentEntry->value.val.lval = val;
    flush_if_flushable(channel);
    ++channel->base.rowCounter;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_intneg(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    otic_unpack_cleaner(&channel->cache.currentEntry->value);
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->cache.currentEntry->value.val.lval);
    channel->cache.currentEntry->value.type = OTIC_TYPE_INT_NEG;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_intpos(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    otic_unpack_cleaner(&channel->cache.currentEntry->value);
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->cache.currentEntry->value.val.lval);
    channel->cache.currentEntry->value.type = OTIC_TYPE_INT_POS;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_double(oticUnpackChannel_t* channel)
{
    // TODO: ASSERT DOUBLE SIZE
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    otic_unpack_cleaner(&channel->cache.currentEntry->value);
    memcpy(&channel->cache.currentEntry->value.val.dval, channel->base.top, sizeof(double));
    channel->base.top += sizeof(double);
    channel->cache.currentEntry->value.type = OTIC_TYPE_DOUBLE;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_float(oticUnpackChannel_t* channel)
{
    // TODO: ASSERT FLOAT SIZE
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    otic_unpack_cleaner(&channel->cache.currentEntry->value);
    memcpy(&channel->cache.currentEntry->value.val.dval, channel->base.top, sizeof(float));
    channel->base.top += sizeof(float);
    channel->cache.currentEntry->value.type = OTIC_TYPE_FLOAT;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_string(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    uint8_t total = *channel->base.top++;
    otic_unpack_cleaner(&channel->cache.currentEntry->value);
    channel->cache.currentEntry->value.val.sval.ptr = malloc((total + 1) * sizeof(char));
    channel->cache.currentEntry->value.val.sval.size = total;
    memcpy((void*)channel->cache.currentEntry->value.val.sval.ptr, channel->base.top, total);
    channel->cache.currentEntry->value.val.sval.ptr[total] = 0;
    channel->cache.currentEntry->value.type = OTIC_TYPE_STRING;
    channel->base.top += total;
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_unmodified(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    flush_if_flushable(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_rawbuffer(oticUnpackChannel_t* channel)
{
    uint32_t size;
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&size);
    channel->base.top += size;
}

static uint8_t read_object(oticUnpackChannel_t* channel, oval_t* oval);

static uint8_t read_array(oticUnpackChannel_t* channel, oval_t* oval)
{
    // TODO: type gotten directly in switch head
    oval->type = OTIC_TYPE_ARRAY;
    uint64_t size;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &size);
    if (!otic_array_init_size(oval, size))
        return 0;
    for (uint32_t counter = 0; counter < oval->val.aval.size; ++counter)
    {
        switch (*channel->base.top++)
        {
            case OTIC_TYPE_INT_POS:
                channel->base.top += leb128_decode_unsigned(channel->base.top, &oval->val.aval.elements[counter].val.lval);
                oval->val.aval.elements[counter].type = OTIC_TYPE_INT_POS;
                break;
            case OTIC_TYPE_INT_NEG:
                channel->base.top += leb128_decode_unsigned(channel->base.top, &oval->val.aval.elements[counter].val.lval);
                oval->val.aval.elements[counter].type = OTIC_TYPE_INT_NEG;
                break;
            case OTIC_TYPE_DOUBLE:
                memcpy(&oval->val.aval.elements[counter].val.dval, channel->base.top, sizeof(double));
                channel->base.top += sizeof(double);
                oval->val.aval.elements[counter].type = OTIC_TYPE_DOUBLE;
                break;
            case OTIC_TYPE_FLOAT:
                memcpy(&oval->val.aval.elements[counter].val.dval, channel->base.top, sizeof(double));
                channel->base.top += sizeof(float);
                oval->val.aval.elements[counter].type = OTIC_TYPE_FLOAT;
                break;
            case OTIC_TYPE_STRING:
                oval->val.aval.elements[counter].val.sval.size = *channel->base.top++;
                oval->val.aval.elements[counter].val.sval.ptr = malloc(oval->val.aval.elements[counter].val.sval.size *
                                                                           sizeof(char));
                memcpy(oval->val.aval.elements[counter].val.sval.ptr, channel->base.top, oval->val.aval.elements[counter].val.sval.size);
                channel->base.top += oval->val.aval.elements[counter].val.sval.size;
                oval->val.aval.elements[counter].type = OTIC_TYPE_STRING;
                break;
            case OTIC_TYPE_ARRAY:
                if (!read_array(channel, &oval->val.aval.elements[counter]))
                    goto fail;
                break;
            case OTIC_TYPE_OBJECT:
                if (!read_object(channel, &oval->val.aval.elements[counter]))
                    goto fail;
                break;
            case OTIC_TYPE_TRUE:
                oval->val.aval.elements[counter].type = OTIC_TYPE_TRUE;
                break;
            case OTIC_TYPE_FALSE:
                oval->val.aval.elements[counter].type = OTIC_TYPE_FALSE;
                break;
            default:
                return 0;
        }
    }
    return 1;
fail:
    // TODO: Memory Cleanup
    return 0;
}

static uint8_t read_object(oticUnpackChannel_t* channel, oval_t* oval)
{
    return 0;
}

OTIC_UNPACK_INLINE
static uint8_t otic_unpack_read_array(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint64_t*)&channel->entryIndex);
    channel->cache.currentEntry = channel->cache.cache[channel->entryIndex];
    read_array(channel, &channel->cache.currentEntry->value);
    flush_if_flushable(channel);
    ++channel->base.rowCounter;
    return 1;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_setTimestamp(oticUnpackChannel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->base.timestampStart);
    channel->base.timestampCurrent = channel->base.timestampStart;
    channel->ts = (double)channel->base.timestampCurrent / OTIC_TS_MULTIPLICATOR;
    if (channel->timeInterval.time_start == 0)
        channel->timeInterval.time_start = channel->ts;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_shiftTimestamp(oticUnpackChannel_t* channel)
{
    uint64_t value;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &value);
    channel->base.timestampCurrent += value;
    channel->ts = (double)channel->base.timestampCurrent / OTIC_TS_MULTIPLICATOR;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_nameAssign(oticUnpackChannel_t* channel)
{
    uint64_t length;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &length);
    channel->cache.currentEntry = otic_unpack_insert_entry(channel, (char*)channel->base.top, (const char*)channel->base.top + length);
    channel->base.top += length;
    channel->cache.currentEntry->value.type = OTIC_TYPE_EOF;
    ++channel->cache.totalEntries;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_eof(oticUnpackChannel_t* channel)
{
    uint64_t expected = 0;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &expected);
    if (expected != channel->base.rowCounter) {
        otic_base_setError(&channel->base, OTIC_ERROR_ROW_COUNT_MISMATCH);
        otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    }
}

OTIC_UNPACK_INLINE
static void otic_unpack_setError(otic_unpack_t* oticUnpack, otic_error_e error)
{
    oticUnpack->error = error;
}

OTIC_UNPACK_INLINE
static otic_error_e otic_unpack_getError(otic_unpack_t* oticUnpack)
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
    otic_base_init(&channel->base, OTIC_BASE_CACHE_SIZE);
    channel->info.channelId = id;
    channel->info.metaData = 0;
    channel->info.parent = parent;
    channel->blockSize = 0;
    channel->cache.currentEntry = 0;
    channel->cache.cache = 0;
    channel->toFetch.ptr = 0;
    channel->toFetch.size = 0;
    channel->cache.cache_allocated = channel->cache.totalEntries = channel->cache.allocationLeft = 0;
    channel->timeInterval.time_start = channel->timeInterval.time_end = 0;
    return 1;
fail:
    otic_unpack_setState(channel->info.parent, OTIC_STATE_ON_ERROR);
    return 0;
}

void otic_unpack_channel_toFetch(oticUnpackChannel_t* channel, const char** values, size_t size)
{
    if (size == 0)
        return;
    channel->toFetch.ptr = malloc(sizeof(size_t) * size);
    channel->toFetch.size = size;
    for (size_t counter = 0; counter < size; ++counter)
        channel->toFetch.ptr[counter] = otic_hashFunction(values[counter]);
}

uint8_t otic_unpack_channel_close(oticUnpackChannel_t* channel)
{
    ZSTD_freeDCtx(channel->dCtx);
    oticUnpackChannel_t* current = channel->info.parent->channels, *before = current;
    while (current)
    {
        if (current == channel) {
            if (current == channel->info.parent->channels) {
                channel->info.parent->channels = current->previous;
            } else {
                before->previous = current->previous;
            }
            size_t counter;
            for (counter = 0; counter < channel->cache.cache_allocated; ++counter)
            {
                if (channel->cache.cache[counter]) {
                    otic_unpack_cleaner(&channel->cache.cache[counter]->value);
                    free(channel->cache.cache[counter]->name);
                    free(channel->cache.cache[counter]->unit);
                    free(channel->cache.cache[counter]);
                }
            }
            otic_base_close(&channel->base);
            free(channel->cache.cache);
            free(channel->toFetch.ptr);
            free(channel->info.metaData);
            free(channel);
            return 1;
        }
        before = current;
        current = current->previous;
    }
    return 0;
}

OTIC_UNPACK_INLINE PURE MUST_CHECK
static int32_t otic_unpack_getMeta(otic_meta_data_t* metaData, uint8_t size, uint32_t value)
{
    uint8_t counter;
    for (counter = 0; counter < size; counter++)
        if (metaData[counter].metaType == value)
            return metaData[counter].channelId;
    return -1;
}

OTIC_UNPACK_INLINE
static uint8_t otic_unpack_getLine(oticUnpackChannel_t* channel)
{
    if (channel->base.top - channel->out  >= channel->blockSize)
    {
        channel->info.parent->current = 0;
        return 1;
    }
    if (*channel->base.top < SMALL_INT_LIMIT) {
        otic_unpack_read_smallint(channel, *channel->base.top++);
    }
    switch (*channel->base.top++)
    {
        case OTIC_TYPE_UNMODIFIED:
            otic_unpack_read_unmodified(channel);
            break;
        case OTIC_TYPE_INT_POS:
            otic_unpack_read_intpos(channel);
            break;
        case OTIC_TYPE_INT_NEG:
            otic_unpack_read_intneg(channel);
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
        case OTIC_TYPE_TRUE:
            otic_unpack_read_true(channel);
            break;
        case OTIC_TYPE_FALSE:
            otic_unpack_read_false(channel);
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
        case OTIC_TYPE_ARRAY:
            otic_unpack_read_array(channel);
            break;
        case OTIC_TYPE_EOF:
            otic_unpack_read_eof(channel);
            return 0;
            break;
        case OTIC_TYPE_STRING:
            otic_unpack_read_string(channel);
            break;
        case OTIC_TYPE_RAWBUFFER:
            otic_unpack_read_rawbuffer(channel);
            break;
    }
    return 1;
}

OTIC_UNPACK_INLINE
static void otic_unpack_parseBlock(oticUnpackChannel_t* channel)
{
    while ((channel->base.top - channel->out) < channel->blockSize)
    {
        if (*channel->base.top < SMALL_INT_LIMIT) {
            otic_unpack_read_smallint(channel, *channel->base.top++);
            continue;
        }
        switch (*channel->base.top++)
        {
            case OTIC_TYPE_UNMODIFIED:
                otic_unpack_read_unmodified(channel);
                break;
            case OTIC_TYPE_INT_POS:
                otic_unpack_read_intpos(channel);
                break;
            case OTIC_TYPE_INT_NEG:
                otic_unpack_read_intneg(channel);
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
            case OTIC_TYPE_TRUE:
                otic_unpack_read_true(channel);
                break;
            case OTIC_TYPE_FALSE:
                otic_unpack_read_false(channel);
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
            case OTIC_TYPE_ARRAY:
                otic_unpack_read_array(channel);
                break;
            case OTIC_TYPE_EOF:
                otic_unpack_read_eof(channel);
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
static uint8_t otic_unpack_read_data(otic_unpack_t* oticUnpack, oticUnpackChannel_t* channel, uint32_t size)
{
    channel->blockSize = size;
    oticUnpack->fetcher(channel->base.cache, size, oticUnpack->fetcherData);
    channel->blockSize = ZSTD_decompressDCtx(
            channel->dCtx, channel->out, OTIC_UNPACK_OUT_SIZE,
            channel->base.cache, size
            );
    if (ZSTD_isError(channel->blockSize)) {
        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
        otic_unpack_setError(oticUnpack, OTIC_ERROR_ZSTD);
        otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
        return 0;
    }
    channel->base.top = channel->out;
    otic_unpack_parseBlock(channel);
    return 1;
}

uint8_t otic_unpack_init(otic_unpack_t* oticUnpack, uint8_t(*fetcher)(uint8_t*, size_t, void*), void* fetcherData, uint8_t(*seeker)(uint32_t, void*), void* seekerData)
{
    oticUnpack->fetcher = fetcher;
    oticUnpack->seeker = seeker;
    oticUnpack->seekerData = seekerData;
    oticUnpack->fetcherData = fetcherData;
    oticUnpack->current = 0;
    otic_header_t header;
    fetcher((uint8_t*)&header, sizeof(header), fetcherData);
    if (memcmp(header.magic, OTIC_MAGIC, OTIC_MAGIC_SIZE) != 0) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_DATA_CORRUPTED);
        goto fail;
    }
    if (header.version > OTIC_VERSION_MAJOR) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_VERSION_UNSUPPORTED);
        goto fail;
    }
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
    oticUnpackChannel_t* current = oticUnpack->channels;
    while (current)
    {
        if (current->info.channelId == id) {
            otic_unpack_setError(oticUnpack, OTIC_ERROR_INVALID_ARGUMENT);
            goto fail;
        }
        current = current->previous;
    }
    oticUnpackChannel_t *temp = malloc(sizeof(oticUnpackChannel_t));
    if (!temp) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    if (!otic_unpack_channel_init(temp, id, flusher, data, oticUnpack)) {
        free(temp);
        goto fail;
    }
    temp->previous = oticUnpack->channels;
    oticUnpack->channels = temp;
    return temp;
fail:
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}

OTIC_UNPACK_INLINE
static oticUnpackChannel_t* findChannel(otic_unpack_t* oticUnpack, uint8_t channelId)
{
    oticUnpackChannel_t* current = oticUnpack->channels;
    while (current != 0)
    {
        if (current->info.channelId == channelId)
            return current;
        current = current->previous;
    }
    return 0;
}

OTIC_UNPACK_INLINE
static void otic_unpack_channel_setState(oticUnpackChannel_t* channel, otic_state_e state)
{
    channel->base.state = state;
}

OTIC_UNPACK_INLINE
static uint8_t otic_unpack_getNext(otic_unpack_t* oticUnpack)
{
    otic_meta_data_t metaData;
    while (1)
    {
        if (oticUnpack->fetcher((uint8_t*)&metaData, sizeof(metaData), oticUnpack->fetcherData) == 0) {
            oticUnpack->current = 0;
            return 0;
        }
        oticUnpackChannel_t *channel = findChannel(oticUnpack, metaData.channelId);
        switch (metaData.metaType)
        {
            case OTIC_META_TYPE_CHANNEL_DEFINE:
                if (channel)
                    otic_unpack_channel_setState(channel, OTIC_STATE_OPENED);
                break;
            case OTIC_META_TYPE_DATA:
            {
                uint32_t size;
                if (!oticUnpack->fetcher((uint8_t*)&size, sizeof(uint32_t), oticUnpack->fetcherData)) {
                    otic_unpack_setError(oticUnpack, OTIC_ERROR_EOF);
                    oticUnpack->current = 0;
                    return 0;
                }
                if (channel) {
                    oticUnpack->current = channel;
                    oticUnpack->current->blockSize = size;
                    oticUnpack->fetcher(channel->base.cache, size, oticUnpack->fetcherData);
                    channel->blockSize = ZSTD_decompressDCtx(
                            channel->dCtx, channel->out, OTIC_UNPACK_OUT_SIZE,
                            channel->base.cache, size
                    );
                    if (ZSTD_isError(channel->blockSize)) {
                        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
                        otic_unpack_setError(oticUnpack, OTIC_ERROR_ZSTD);
                        otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
                        oticUnpack->current = 0;
                        return 0;
                    }
                    channel->base.top = channel->out;
                    return otic_unpack_getLine(channel);
                }
                if (oticUnpack->seeker)
                    oticUnpack->seeker(size, oticUnpack->seekerData);
                else {
                    uint8_t ptr[size];
                    oticUnpack->fetcher(ptr, size, oticUnpack->fetcherData);
                }
            }
                break;
            case OTIC_META_TYPE_COMPRESSION_METHOD:
            {
                uint8_t value;
                if (!oticUnpack->fetcher(&value, 1, oticUnpack->fetcherData)) {
                    otic_unpack_setError(oticUnpack, OTIC_ERROR_EOF);
                    oticUnpack->current = 0;
                    return 0;
                }
            }
                break;
            case OTIC_META_TYPE_CHANNEL_TYPE:
            {
                uint8_t read;
                if (!oticUnpack->fetcher(&read, 1, oticUnpack->fetcherData)) {
                    otic_unpack_setError(oticUnpack, OTIC_ERROR_EOF);
                    oticUnpack->current = 0;
                    return 0;
                }
                if (channel)
                    channel->info.channelType = read;
            }
                return 1;
            case OTIC_META_TYPE_CHUNK_SIZE:
            {
                uint32_t size;
                if (!oticUnpack->fetcher((uint8_t *) &size, sizeof(uint32_t), oticUnpack->fetcherData)) {
                    otic_unpack_setError(oticUnpack, OTIC_ERROR_EOF);
                    oticUnpack->current = 0;
                    return 0;
                }
                if (!channel)
                    return 1;
                if (channel->base.cache)
                    free(channel->base.cache);
                channel->base.cache = malloc(size);
                if (!channel->base.cache) {
                    otic_unpack_setError(oticUnpack, OTIC_ERROR_ALLOCATION_FAILURE);
                    oticUnpack->current = 0;
                    return 0;
                }
            }
                break;
            default:
                otic_unpack_setError(oticUnpack, OTIC_ERROR_DATA_CORRUPTED);
                oticUnpack->current = 0;
                return 0;
        }
    }
}

uint8_t otic_unpack_generate(otic_unpack_t* oticUnpack)
{
    return oticUnpack->current ? otic_unpack_getLine(oticUnpack->current) : otic_unpack_getNext(oticUnpack);
}

uint8_t otic_unpack_parse(otic_unpack_t* oticUnpack) {
    otic_meta_data_t metaData;
    if (oticUnpack->fetcher((uint8_t *)&metaData, sizeof(metaData), oticUnpack->fetcherData) == 0) {
        return 0;
    }
    oticUnpackChannel_t* channel = findChannel(oticUnpack, metaData.channelId);
    switch (metaData.metaType) {
        case OTIC_META_TYPE_CHANNEL_DEFINE:
            if (channel)
                otic_unpack_channel_setState(channel, OTIC_STATE_OPENED);
            break;
        case OTIC_META_TYPE_DATA:
        {
            uint32_t size;
            if (!oticUnpack->fetcher((uint8_t*)&size, sizeof(uint32_t), oticUnpack->fetcherData)) {
                otic_unpack_setError(oticUnpack, OTIC_ERROR_EOF);
                goto fail;
            }
            if (channel)
                return otic_unpack_read_data(oticUnpack, channel, size);
            if (oticUnpack->seeker)
                oticUnpack->seeker(size, oticUnpack->seekerData);
            else {
                uint8_t ptr[size];
                oticUnpack->fetcher(ptr, size, oticUnpack->fetcherData);
            }
        }
        break;
        case OTIC_META_TYPE_COMPRESSION_METHOD:
        {
            uint8_t value;
            if (!oticUnpack->fetcher(&value, 1, oticUnpack->fetcherData)) {
                otic_unpack_setError(oticUnpack, OTIC_ERROR_EOF);
                goto fail;
            }
        }
            break;
        case OTIC_META_TYPE_CHANNEL_TYPE:
        {
            uint8_t read;
            if (!oticUnpack->fetcher(&read, 1, oticUnpack->fetcherData)) {
                otic_unpack_setError(oticUnpack, OTIC_ERROR_EOF);
                goto fail;
            }
            if (channel)
                channel->info.channelType = read;
        }
            break;
        case OTIC_META_TYPE_CHUNK_SIZE:
        {
            uint32_t size;
            if (!oticUnpack->fetcher((uint8_t *) &size, sizeof(uint32_t), oticUnpack->fetcherData)) {
                otic_unpack_setError(oticUnpack, OTIC_ERROR_EOF);
                goto fail;
            }
            if (!channel)
                return 1;
            if (channel->base.cache)
                free(channel->base.cache);
            channel->base.cache = malloc(size);
            if (!channel->base.cache) {
                otic_unpack_setError(oticUnpack, OTIC_ERROR_ALLOCATION_FAILURE);
                goto fail;
            }
        }
            break;
        default:
            otic_unpack_setError(oticUnpack, OTIC_ERROR_DATA_CORRUPTED);
            goto fail;
    }
    return 1;
fail:
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_unpack_closeChannel(otic_unpack_t* oticUnpack, uint8_t id)
{
    oticUnpackChannel_t* current = oticUnpack->channels, *before = oticUnpack->channels;
    while (current)
    {
        if (current->info.channelId == id) {
            if (current == oticUnpack->channels) {
                oticUnpack->channels = current->previous;
            } else {
                before->previous = current->previous;
            }
            free(current);
            return 1;
        }
        before = current;
        current = current->previous;
    }
    otic_unpack_setError(oticUnpack, OTIC_ERROR_INVALID_ARGUMENT);
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_unpack_getTotalAmountOfChannel(const otic_unpack_t* oticUnpack)
{
    uint8_t counter = 0;
    for (oticUnpackChannel_t* current = oticUnpack->channels; current; current = current->previous, ++counter);
    return counter;
}

uint8_t otic_unpack_close(otic_unpack_t* oticUnpack)
{
    oticUnpackChannel_t* current = oticUnpack->channels;
    while (current) {
        oticUnpackChannel_t* temp = current->previous;
        if (!otic_unpack_channel_close(current))
            goto fail;
        current = temp;
    }
    otic_unpack_setState(oticUnpack, OTIC_STATE_CLOSED);
    return 1;
fail:
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}
