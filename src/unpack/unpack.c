#include <fenv.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <zstd.h>
#include "unpack.h"
#include "config/config.h"

#define OTIC_UNPACK_INLINE_ALL_STATIC 1

#if OTIC_UNPACK_INLINE_ALL_STATIC
#define OTIC_UNPACK_INLINE inline
#else
#define OTIC_UNPACK_INLINE
#endif

typedef void (*parser)(otic_unpack_channel_t* unpack);
typedef struct
{
    parser parserFunc;
    parser printerFunc;
} parser_printer;
parser_printer parsers[];

// TODO: USE INDEX instead of total entries
OTIC_UNPACK_INLINE
static otic_unpack_entry_t* otic_unpack_insert_entry(otic_unpack_channel_t* channel, char* value, const char* end) {
    if (channel->cache_t.allocationLeft == 0) {
        channel->cache_t.cache = realloc(channel->cache_t.cache, sizeof(*channel->cache_t.cache) * (OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE + channel->cache_t.cache_allocated));
        channel->cache_t.allocationLeft = OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE;
        channel->cache_t.cache_allocated += OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE;
    }
    char *ptr = value;
    while (ptr <= end)
    {
        if (*ptr == ':'){
            *ptr = 0;
            ptr++;
            break;
        }
        ptr++;
    }
    channel->cache_t.cache[channel->cache_t.totalEntries] = malloc(sizeof(otic_unpack_entry_t));
    uint8_t lengthValue = (ptr - value);
    channel->cache_t.cache[channel->cache_t.totalEntries]->index = channel->cache_t.totalEntries;
    channel->cache_t.cache[channel->cache_t.totalEntries]->name = malloc((lengthValue + 1) * sizeof(char));
    memcpy(channel->cache_t.cache[channel->cache_t.totalEntries]->name, value, lengthValue);
    channel->cache_t.cache[channel->cache_t.totalEntries]->name[lengthValue] = 0;
    channel->cache_t.cache[channel->cache_t.totalEntries]->unit = malloc((end - ptr + 1u) * sizeof(char));
    memcpy(channel->cache_t.cache[channel->cache_t.totalEntries]->unit, ptr,  end - ptr);
    channel->cache_t.cache[channel->cache_t.totalEntries]->last_value.string_value.size = 0;
    channel->cache_t.cache[channel->cache_t.totalEntries]->last_value.string_value.value = 0;
    channel->cache_t.allocationLeft--;
    return channel->cache_t.cache[channel->cache_t.totalEntries];
}

OTIC_UNPACK_INLINE
static void otic_unpack_channel_flush(otic_unpack_channel_t* channel)
{
    channel->flusher((uint8_t*)channel->result.content, channel->result.top - channel->result.content);
    channel->result.top = channel->result.content;
}

OTIC_UNPACK_INLINE
static void otic_unpack_flush_if_flushable(otic_unpack_channel_t* channel)
{
    if (channel->result.top > channel->result.limit)
        otic_unpack_channel_flush(channel);
}

OTIC_UNPACK_INLINE
static void otic_unpack_printer_i(otic_unpack_channel_t* channel)
{
    channel->result.top += sprintf(channel->result.top, "%lf\t%s\t%s\t%u\n", channel->doubleTs, channel->cache_t.currentEntry->name, channel->cache_t.currentEntry->unit, channel->cache_t.currentEntry->last_value.int_value);
    otic_unpack_flush_if_flushable(channel);
}

OTIC_UNPACK_INLINE
static void otic_unpack_printer_in(otic_unpack_channel_t* channel)
{
    channel->result.top += sprintf(channel->result.top, "%lf\t%s\t%s\t-%u\n", channel->doubleTs, channel->cache_t.currentEntry->name, channel->cache_t.currentEntry->unit, channel->cache_t.currentEntry->last_value.int_value);
    otic_unpack_flush_if_flushable(channel);
}

OTIC_UNPACK_INLINE
static void otic_unpack_printer_d(otic_unpack_channel_t* channel)
{
    channel->result.top += sprintf(channel->result.top, "%lf\t%s\t%s\t%lf\n", channel->doubleTs, channel->cache_t.currentEntry->name, channel->cache_t.currentEntry->unit, channel->cache_t.currentEntry->last_value.double_value);
    otic_unpack_flush_if_flushable(channel);
}


OTIC_UNPACK_INLINE
static void otic_unpack_printer_s(otic_unpack_channel_t* channel)
{
    channel->result.top += sprintf(channel->result.top, "%lf\t%s\t%s\t%s\n", channel->doubleTs, channel->cache_t.currentEntry->name, channel->cache_t.currentEntry->unit, channel->cache_t.currentEntry->last_value.string_value.value);
    otic_unpack_flush_if_flushable(channel);
}

OTIC_UNPACK_INLINE
static void otic_unpack_printer_n(otic_unpack_channel_t* channel)
{
    channel->result.top += sprintf(channel->result.top, "%lf\t%s\t%s\t\n", channel->doubleTs, channel->cache_t.currentEntry->name, channel->cache_t.currentEntry->unit);
    otic_unpack_flush_if_flushable(channel);
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_null(otic_unpack_channel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    otic_unpack_printer_n(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_empty(otic_unpack_channel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_int32neg(otic_unpack_channel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache_t.currentEntry = channel->cache_t.cache[channel->entryIndex];
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->cache_t.currentEntry->last_value.int_value);
    channel->cache_t.currentEntry->type = OTIC_TYPE_INT32_NEG;
    otic_unpack_printer_in(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_int32pos(otic_unpack_channel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache_t.currentEntry = channel->cache_t.cache[channel->entryIndex];
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->cache_t.currentEntry->last_value.int_value);
    channel->cache_t.currentEntry->type = OTIC_TYPE_INT32_POS;
    otic_unpack_printer_i(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_double(otic_unpack_channel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache_t.currentEntry = channel->cache_t.cache[channel->entryIndex];
    memcpy(&channel->cache_t.currentEntry->last_value.double_value, channel->base.top, sizeof(double));
    channel->base.top += sizeof(double);
    channel->cache_t.currentEntry->type = OTIC_TYPE_DOUBLE;
    otic_unpack_printer_d(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min1float(otic_unpack_channel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min2float(otic_unpack_channel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min3float(otic_unpack_channel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_float(otic_unpack_channel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_meddouble(otic_unpack_channel_t* channel)
{
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_string(otic_unpack_channel_t* channel)
{
    uint32_t total = 0;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache_t.currentEntry = channel->cache_t.cache[channel->entryIndex];
    total = *channel->base.top++;
    if (channel->cache_t.currentEntry->last_value.string_value.size < total)
        channel->cache_t.currentEntry->last_value.string_value.value = realloc(channel->cache_t.currentEntry->last_value.string_value.value, (channel->cache_t.currentEntry->last_value.string_value.size + 1) * sizeof(char));
    channel->cache_t.currentEntry->last_value.string_value.size = total;
    memcpy(channel->cache_t.currentEntry->last_value.string_value.value, channel->base.top, channel->cache_t.currentEntry->last_value.string_value.size);
    channel->cache_t.currentEntry->last_value.string_value.value[channel->cache_t.currentEntry->last_value.string_value.size] = 0;
    channel->cache_t.currentEntry->type = OTIC_TYPE_STRING;
    channel->base.top += total;
    otic_unpack_printer_s(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_unmodified(otic_unpack_channel_t* channel)
{
    channel->base.top += leb128_decode_unsigned(channel->base.top, &channel->entryIndex);
    channel->cache_t.currentEntry = channel->cache_t.cache[channel->entryIndex];
    parsers[channel->cache_t.currentEntry->type].printerFunc(channel);
    channel->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_rawbuffer(otic_unpack_channel_t* channel)
{
    uint32_t size;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &size);
    channel->base.top += size;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_setTimestamp(otic_unpack_channel_t* channel)
{

    channel->base.top += leb128_decode_unsigned(channel->base.top, (uint32_t*)&channel->base.timestamp_start);
    channel->base.timestamp_current = channel->base.timestamp_start;
    channel->doubleTs = (double)channel->base.timestamp_current / OTIC_TS_MULTIPLICATOR;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_shiftTimestamp(otic_unpack_channel_t* channel)
{
    uint32_t value;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &value);
    channel->base.timestamp_current += value;
    channel->doubleTs = (double)channel->base.timestamp_current / OTIC_TS_MULTIPLICATOR;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_fileVersion(otic_unpack_channel_t* channel)
{
    if (*channel->base.top++ < OTIC_VERSION_MAJOR)
        otic_base_setError(&channel->base, OTIC_ERROR_VERSION_UNSUPPORTED);
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_nameAssign(otic_unpack_channel_t* channel)
{
    uint32_t length = 0;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &length);
    channel->cache_t.currentEntry = otic_unpack_insert_entry(channel, (char*)channel->base.top, (const char*)channel->base.top + length);
    channel->base.top += length;
    channel->cache_t.totalEntries++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_eof(otic_unpack_channel_t* channel)
{
    uint32_t expected;
    channel->base.top += leb128_decode_unsigned(channel->base.top, &expected);
    if (expected != channel->base.rowCounter)
        otic_base_setError(&channel->base, OTIC_ERROR_ROW_COUNT_MISMATCH);
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
static void otic_unpack_read_metadata(otic_unpack_t* oticUnpack)
{
    uint8_t size = 0;
    oticUnpack->fetcher(&size, sizeof(uint8_t));
    otic_meta_data_t oticMetaData[size];
    oticUnpack->fetcher((uint8_t *)&oticMetaData, sizeof(oticMetaData));
    int32_t arg;
    if ((arg = otic_unpack_getMeta(oticMetaData, size, OTIC_META_TYPE_CHANNEL_DEFINE)) == -1)
        return;
    uint8_t counter;

    for (counter = 0; counter < oticUnpack->totalChannels; counter++)
        if (oticUnpack->channels[counter]->info.channelId == arg){
            oticUnpack->channels[counter]->info.metaData = malloc(size * sizeof(otic_meta_data_t));
            return;
        }
}

parser_printer parsers[] = {
        {otic_unpack_read_null, otic_unpack_printer_n},
        {otic_unpack_read_empty, otic_unpack_printer_n},
        {otic_unpack_read_int32neg, otic_unpack_printer_in},
        {otic_unpack_read_int32pos, otic_unpack_printer_i},
        {otic_unpack_read_double, otic_unpack_printer_d},
        {otic_unpack_read_min1float, 0},
        {otic_unpack_read_min2float, 0},
        {otic_unpack_read_min3float, 0},
        {otic_unpack_read_float, otic_unpack_printer_d},
        {otic_unpack_read_meddouble, 0},
        {otic_unpack_read_string, otic_unpack_printer_s},
        {otic_unpack_read_unmodified, 0},
        {otic_unpack_read_rawbuffer, 0},
        {otic_unpack_read_setTimestamp, 0},
        {otic_unpack_read_shiftTimestamp, 0},
        {otic_unpack_read_fileVersion, 0},
        {otic_unpack_read_nameAssign, 0},
        {otic_unpack_read_eof, 0},
};

uint8_t otic_unpack_parseBlock(otic_unpack_channel_t* channel)
{
    while (channel->base.top - channel->out < channel->blockSize)
        parsers[*channel->base.top++].parserFunc(channel);
    return 1;
fail:
    return 0;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_data(otic_unpack_t* oticUnpack)
{
    otic_payload_t payload;
    oticUnpack->fetcher((uint8_t*)&payload, sizeof(payload));
    uint8_t counter;
    for (counter = 0; counter < oticUnpack->totalChannels; counter++)
    {
        if (oticUnpack->channels[counter]->info.channelId == payload.channelId){
            oticUnpack->channels[counter]->base.timestamp_start = oticUnpack->channels[counter]->base.timestamp_current = payload.startTimestamp;
            oticUnpack->channels[counter]->doubleTs = (double)oticUnpack->channels[counter]->base.timestamp_start/ OTIC_TS_MULTIPLICATOR;
            oticUnpack->channels[counter]->blockSize = payload.dataLen;
            oticUnpack->fetcher(oticUnpack->channels[counter]->base.cache, payload.dataLen);
            oticUnpack->channels[counter]->blockSize = ZSTD_decompressDCtx(oticUnpack->channels[counter]->dCtx, oticUnpack->channels[counter]->out, OTIC_UNPACK_OUT_SIZE, oticUnpack->channels[counter]->base.cache, payload.dataLen);
            if (ZSTD_isError(oticUnpack->channels[counter]->blockSize)){
                otic_base_setError(&oticUnpack->channels[counter]->base, OTIC_ERROR_ZSTD);
                return;
            }
            oticUnpack->channels[counter]->base.top = oticUnpack->channels[counter]->out;
            otic_unpack_parseBlock(oticUnpack->channels[counter]);
            return;
        }
    }
    if (oticUnpack->seeker){
        oticUnpack->seeker(payload.dataLen);
    } else {
        uint8_t ptr[payload.dataLen];
        oticUnpack->fetcher(ptr, payload.dataLen);
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

uint8_t otic_unpack_init(otic_unpack_t* oticUnpack, uint8_t(*fetcher)(uint8_t*, size_t), uint8_t(*seeker)(uint32_t))
{
    if (!oticUnpack)
        return 0;
    if (!fetcher){
        otic_unpack_setError(oticUnpack, OTIC_ERROR_INVALID_POINTER);
        goto fail;
    }
    oticUnpack->fetcher = fetcher;
    oticUnpack->seeker = seeker;
    otic_header_t header;
    fetcher((uint8_t*)&header, sizeof(header));
    if (strncmp((const char*)header.magic, "OC\x07\xFF", OTIC_MAGIC_SIZE) != 0) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_DATA_CORRUPTED);
        goto fail;
    }
    if (header.version < OTIC_VERSION_MAJOR) {
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

uint8_t otic_unpack_defineChannel(otic_unpack_t* oticUnpack, uint8_t id, uint8_t(*flusher)(uint8_t*, size_t))
{
    if (!oticUnpack)
        return 0;
    if (!flusher) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_INVALID_POINTER);
        goto fail;
    }
    if (oticUnpack->state != OTIC_STATE_OPENED) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_AT_INVALID_STATE);
        goto fail;
    }
    otic_unpack_channel_t** ptr = realloc(oticUnpack->channels, sizeof(otic_unpack_channel_t *) * (oticUnpack->totalChannels + 1));
    if (!ptr){
        otic_unpack_setError(oticUnpack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    oticUnpack->channels = ptr;
    oticUnpack->channels[oticUnpack->totalChannels] = malloc(sizeof(otic_unpack_channel_t));
    if (!oticUnpack->channels) {
        otic_unpack_setError(oticUnpack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    if (!(oticUnpack->channels[oticUnpack->totalChannels]->dCtx = ZSTD_createDCtx())){
        otic_unpack_setError(oticUnpack, OTIC_ERROR_ZSTD);
        goto fail;
    }
    oticUnpack->channels[oticUnpack->totalChannels]->info.channelId = id;
    oticUnpack->channels[oticUnpack->totalChannels]->info.parent = oticUnpack;
    oticUnpack->channels[oticUnpack->totalChannels]->flusher = flusher;
    oticUnpack->channels[oticUnpack->totalChannels]->blockSize = 0;
    oticUnpack->channels[oticUnpack->totalChannels]->cache_t.currentEntry = 0;
    oticUnpack->channels[oticUnpack->totalChannels]->result.top = oticUnpack->channels[oticUnpack->totalChannels]->result.content;
    oticUnpack->channels[oticUnpack->totalChannels]->result.limit = &oticUnpack->channels[oticUnpack->totalChannels]->result.content[OTIC_UNPACK_RESULT_OUTSIZE - OTIC_UNPACK_OUTPUT_LIMIT];
    oticUnpack->channels[oticUnpack->totalChannels]->cache_t.cache_allocated = oticUnpack->channels[oticUnpack->totalChannels]->cache_t.totalEntries = oticUnpack->channels[oticUnpack->totalChannels]->cache_t.allocationLeft = 0;
    oticUnpack->totalChannels++;
    return 1;
fail:
    otic_unpack_setState(oticUnpack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_unpack_channel_close(otic_unpack_channel_t* channel)
{
    otic_unpack_channel_flush(channel);
    ZSTD_freeDCtx(channel->dCtx);
    size_t counter;
    for (counter = 0; counter < channel->cache_t.cache_allocated; ++counter)
    {
        if (channel->cache_t.cache[counter]){
            if (channel->cache_t.cache[counter]->last_value.string_value.size != 0) {
                free(channel->cache_t.cache[counter]->last_value.string_value.value);
            }
            free(channel->cache_t.cache[counter]);
        }
    }
    channel->cache_t.cache_allocated = channel->cache_t.allocationLeft == 0;
    channel->cache_t.currentEntry = 0;
    otic_base_setState(&channel->base, OTIC_STATE_CLOSED);
    return 1;
}


uint8_t otic_unpack_closeChannel(otic_unpack_t* oticUnpack,uint8_t id)
{
    uint8_t counter;
    for (counter = 0; counter < oticUnpack->totalChannels; counter++)
    {
        if (oticUnpack->channels[counter]->info.channelId == id){
            otic_unpack_channel_flush(oticUnpack->channels[counter]);
            ZSTD_freeDCtx(oticUnpack->channels[counter]->dCtx);
            size_t counter2;
            for (counter2 = 0; counter2 < oticUnpack->channels[counter]->cache_t.cache_allocated; counter2++)
            {
                if (oticUnpack->channels[counter]->cache_t.cache[counter2]->last_value.string_value.value)
                    free(oticUnpack->channels[counter]->cache_t.cache[counter2]->last_value.string_value.value);
                free(oticUnpack->channels[counter]->cache_t.cache);
            }
            free(oticUnpack->channels[counter]->cache_t.cache);
            oticUnpack->channels[counter]->cache_t.cache_allocated = oticUnpack->channels[counter]->cache_t.allocationLeft == 0;
            oticUnpack->channels[counter]->cache_t.currentEntry = 0;
            otic_base_setState(&oticUnpack->channels[counter]->base, OTIC_STATE_CLOSED);
            return 1;
        }
    }
    return 0;
}

uint8_t otic_unpack_parse(otic_unpack_t* oticUnpack) {
    if (!oticUnpack)
        return 0;
    static uint8_t value;
    oticUnpack->fetcher(&value, 1);
    if (value == OTIC_TYPE_METADATA)
        otic_unpack_read_metadata(oticUnpack);
    else if (value == OTIC_TYPE_DATA)
        otic_unpack_read_data(oticUnpack);
    return 1;
}

uint8_t otic_unpack_close(otic_unpack_t* oticUnpack)
{
    uint8_t counter;
    for (counter = 0; counter < oticUnpack->totalChannels; counter++)
    {
        otic_unpack_channel_close(oticUnpack->channels[counter]);
        free(oticUnpack->channels[counter]);
    }
    free(oticUnpack->channels);
    otic_unpack_setState(oticUnpack, OTIC_STATE_CLOSED);
    return 1;
}
