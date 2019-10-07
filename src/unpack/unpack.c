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

static uint32_t entry_index;

typedef void (*parser)(otic_unpack_t* unpack);
typedef struct
{
    parser parserFunc;
    parser printerFunc;
} parser_printer;
parser_printer parsers[];

// TODO: USE INDEX instead of total entries
OTIC_UNPACK_INLINE
static otic_unpack_entry_t* otic_unpack_insert_entry(otic_unpack_t* oticUnpack, char* value, const char* end) {
    if (oticUnpack->cache_t.allocationLeft == 0) {
        oticUnpack->cache_t.cache = realloc(oticUnpack->cache_t.cache, sizeof(*oticUnpack->cache_t.cache) * (OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE + oticUnpack->cache_t.cache_allocated));
        oticUnpack->cache_t.allocationLeft = OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE;
        oticUnpack->cache_t.cache_allocated += OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE;
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
    oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries] = malloc(sizeof(otic_unpack_entry_t));
    uint8_t lengthValue = (ptr - value);
    oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries]->index = oticUnpack->cache_t.totalEntries;
    oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries]->name = malloc((lengthValue + 1) * sizeof(char));
    memcpy(oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries]->name, value, lengthValue);
    oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries]->name[lengthValue] = 0;
    oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries]->unit = malloc((end - ptr + 1u) * sizeof(char));
    memcpy(oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries]->unit, ptr,  end - ptr);
    oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries]->last_value.string_value.size = 0;
    oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries]->last_value.string_value.value = 0;
    oticUnpack->cache_t.allocationLeft--;
    return oticUnpack->cache_t.cache[oticUnpack->cache_t.totalEntries];
}

OTIC_UNPACK_INLINE
void otic_unpack_flush(otic_unpack_t* oticUnpack)
{
    oticUnpack->flusher((uint8_t*)oticUnpack->result.content, oticUnpack->result.top - oticUnpack->result.content);
    oticUnpack->result.top = oticUnpack->result.content;
}

OTIC_UNPACK_INLINE
static void otic_unpack_flush_if_flushable(otic_unpack_t* oticUnpack)
{
    if (oticUnpack->result.top > oticUnpack->result.limit)
        otic_unpack_flush(oticUnpack);
}

OTIC_UNPACK_INLINE
static void otic_unpack_printer_i(otic_unpack_t* oticUnpack)
{
    oticUnpack->result.top += sprintf(oticUnpack->result.top, "%lf\t%s\t%s\t%u\n", oticUnpack->doubleTs, oticUnpack->cache_t.currentEntry->name, oticUnpack->cache_t.currentEntry->unit, oticUnpack->cache_t.currentEntry->last_value.int_value);
    otic_unpack_flush_if_flushable(oticUnpack);
}

OTIC_UNPACK_INLINE
static void otic_unpack_printer_in(otic_unpack_t* oticUnpack)
{
    oticUnpack->result.top += sprintf(oticUnpack->result.top, "%lf\t%s\t%s\t-%u\n", oticUnpack->doubleTs, oticUnpack->cache_t.currentEntry->name, oticUnpack->cache_t.currentEntry->unit, oticUnpack->cache_t.currentEntry->last_value.int_value);
    otic_unpack_flush_if_flushable(oticUnpack);
}

OTIC_UNPACK_INLINE
static void otic_unpack_printer_d(otic_unpack_t* oticUnpack)
{
    oticUnpack->result.top += sprintf(oticUnpack->result.top, "%lf\t%s\t%s\t%lf\n", oticUnpack->doubleTs, oticUnpack->cache_t.currentEntry->name, oticUnpack->cache_t.currentEntry->unit, oticUnpack->cache_t.currentEntry->last_value.double_value);
    otic_unpack_flush_if_flushable(oticUnpack);
}


OTIC_UNPACK_INLINE
static void otic_unpack_printer_s(otic_unpack_t* oticUnpack)
{
    oticUnpack->result.top += sprintf(oticUnpack->result.top, "%lf\t%s\t%s\t%s\n", oticUnpack->doubleTs, oticUnpack->cache_t.currentEntry->name, oticUnpack->cache_t.currentEntry->unit, oticUnpack->cache_t.currentEntry->last_value.string_value.value);
    otic_unpack_flush_if_flushable(oticUnpack);
}

OTIC_UNPACK_INLINE
static void otic_unpack_printer_n(otic_unpack_t* oticUnpack)
{
    oticUnpack->result.top += sprintf(oticUnpack->result.top, "%lf\t%s\t%s\t\n", oticUnpack->doubleTs, oticUnpack->cache_t.currentEntry->name, oticUnpack->cache_t.currentEntry->unit);
    otic_unpack_flush_if_flushable(oticUnpack);
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_null(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &entry_index);
    otic_unpack_printer_n(oticUnpack);
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_empty(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_int32neg(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &entry_index);
    oticUnpack->cache_t.currentEntry = oticUnpack->cache_t.cache[entry_index];
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &oticUnpack->cache_t.currentEntry->last_value.int_value);
    oticUnpack->cache_t.currentEntry->type = OTIC_TYPE_INT32_NEG;
    otic_unpack_printer_in(oticUnpack);
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_int32pos(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &entry_index);
    oticUnpack->cache_t.currentEntry = oticUnpack->cache_t.cache[entry_index];
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &oticUnpack->cache_t.currentEntry->last_value.int_value);
    oticUnpack->cache_t.currentEntry->type = OTIC_TYPE_INT32_POS;
    otic_unpack_printer_i(oticUnpack);
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_double(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &entry_index);
    oticUnpack->cache_t.currentEntry = oticUnpack->cache_t.cache[entry_index];
    memcpy(&oticUnpack->cache_t.currentEntry->last_value.double_value, oticUnpack->base.top, sizeof(double));
    oticUnpack->base.top += sizeof(double);
    oticUnpack->cache_t.currentEntry->type = OTIC_TYPE_DOUBLE;
    otic_unpack_printer_d(oticUnpack);
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min1float(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min2float(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_min3float(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_float(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_meddouble(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_string(otic_unpack_t* oticUnpack)
{
    uint32_t total = 0;
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &entry_index);
    oticUnpack->cache_t.currentEntry = oticUnpack->cache_t.cache[entry_index];
    total = *oticUnpack->base.top++;
    if (oticUnpack->cache_t.currentEntry->last_value.string_value.size < total)
        oticUnpack->cache_t.currentEntry->last_value.string_value.value = realloc(oticUnpack->cache_t.currentEntry->last_value.string_value.value, (oticUnpack->cache_t.currentEntry->last_value.string_value.size + 1) * sizeof(char));
    oticUnpack->cache_t.currentEntry->last_value.string_value.size = total;
    memcpy(oticUnpack->cache_t.currentEntry->last_value.string_value.value, oticUnpack->base.top, oticUnpack->cache_t.currentEntry->last_value.string_value.size);
    oticUnpack->cache_t.currentEntry->last_value.string_value.value[oticUnpack->cache_t.currentEntry->last_value.string_value.size] = 0;
    oticUnpack->cache_t.currentEntry->type = OTIC_TYPE_STRING;
    oticUnpack->base.top += total;
    otic_unpack_printer_s(oticUnpack);
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_unmodified(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &entry_index);
    oticUnpack->cache_t.currentEntry = oticUnpack->cache_t.cache[entry_index];
    parsers[oticUnpack->cache_t.currentEntry->type].printerFunc(oticUnpack);
    oticUnpack->base.rowCounter++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_rawbuffer(otic_unpack_t* oticUnpack)
{
    uint32_t size;
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &size);
    oticUnpack->base.top += size;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_setTimestamp(otic_unpack_t* oticUnpack)
{
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, (uint32_t*)&oticUnpack->base.timestamp_start);
    oticUnpack->base.timestamp_current = oticUnpack->base.timestamp_start;
    oticUnpack->doubleTs = (double)oticUnpack->base.timestamp_current / OTIC_TS_MULTIPLICATOR;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_shiftTimestamp(otic_unpack_t* oticUnpack)
{
    uint32_t value;
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &value);
    oticUnpack->base.timestamp_current += value;
    oticUnpack->doubleTs = (double)oticUnpack->base.timestamp_current / OTIC_TS_MULTIPLICATOR;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_fileVersion(otic_unpack_t* oticUnpack)
{
    if (*oticUnpack->base.top++ < OTIC_VERSION_MAJOR)
        otic_base_setError(&oticUnpack->base, OTIC_ERROR_VERSION_UNSUPPORTED);
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_nameAssign(otic_unpack_t* oticUnpack)
{
    uint32_t length = 0;
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &length);
    oticUnpack->cache_t.currentEntry = otic_unpack_insert_entry(oticUnpack, (char*)oticUnpack->base.top, (const char*)oticUnpack->base.top + length);
    oticUnpack->base.top += length;
    oticUnpack->cache_t.totalEntries++;
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_eof(otic_unpack_t* oticUnpack)
{
    uint32_t expected;
    oticUnpack->base.top += leb128_decode_unsigned(oticUnpack->base.top, &expected);
    if (expected != oticUnpack->base.rowCounter)
        otic_base_setError(&oticUnpack->base, OTIC_ERROR_ROW_COUNT_MISMATCH);
}

OTIC_UNPACK_INLINE
static void otic_unpack_read_metadata(otic_unpack_t* oticUnpack)
{
    uint8_t size = *oticUnpack->base.top++;
    uint8_t buffered[UINT8_MAX];
    memcpy(buffered, oticUnpack->base.top, size);
    oticUnpack->base.top += size;
    buffered[size] = 0;
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
        {otic_unpack_read_metadata, 0}
};

OTIC_UNPACK_INLINE
static uint8_t otic_unpack_readBlock(otic_unpack_t* oticUnpack)
{
    uint32_t toRead = 0;
    oticUnpack->fetcher((uint8_t*)&toRead, sizeof(uint32_t));
    oticUnpack->fetcher(oticUnpack->base.cache, toRead);
    oticUnpack->blockSize = ZSTD_decompressDCtx(oticUnpack->dCtx, oticUnpack->out, OTIC_UNPACK_OUT_SIZE, oticUnpack->base.cache, toRead);
    if (ZSTD_isError(oticUnpack->blockSize)){
        otic_base_setError(&oticUnpack->base, OTIC_ERROR_ZSTD);
        return 0;
    }
    oticUnpack->base.top = oticUnpack->out;
    return 1;
}

uint8_t otic_unpack_init(otic_unpack_t* oticUnpack, uint8_t(*fetcher)(uint8_t*, size_t), uint8_t(*flusher)(uint8_t*, size_t))
{
    if (!otic_base_init(&oticUnpack->base))
        goto fail;
    if (!fetcher || !flusher) {
        otic_base_setError(&oticUnpack->base, OTIC_ERROR_INVALID_POINTER);
        goto fail;
    }
    if (!(oticUnpack->dCtx = ZSTD_createDCtx())){
        otic_base_setError(&oticUnpack->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
    oticUnpack->fetcher = fetcher;
    oticUnpack->flusher = flusher;
    oticUnpack->blockSize = 0;
    oticUnpack->cache_t.currentEntry = 0;
    oticUnpack->result.top = oticUnpack->result.content;
    oticUnpack->result.limit = &oticUnpack->result.content[OTIC_UNPACK_RESULT_OUTSIZE - OTIC_UNPACK_OUTPUT_LIMIT];
#pragma STDC FENV_ACCESS ON
    fesetround(FE_UPWARD);
    oticUnpack->cache_t.cache_allocated = oticUnpack->cache_t.totalEntries = oticUnpack->cache_t.allocationLeft = 0;
    if (!otic_unpack_readBlock(oticUnpack))
        goto fail;
    if (strcmp("otic", (char*)oticUnpack->base.top) != 0){
        otic_base_setError(&oticUnpack->base, OTIC_ERROR_DATA_CORRUPTED);
        goto fail;
    }
    oticUnpack->base.top += 5;
    otic_unpack_parseBlock(oticUnpack);
    return 1;
fail:
    return 0;
}

uint8_t otic_unpack_parseBlock(otic_unpack_t* oticUnpack)
{
    // TODO: Wohldefinitheit
    while (oticUnpack->base.top - oticUnpack->out < oticUnpack->blockSize)
        parsers[*oticUnpack->base.top++].parserFunc(oticUnpack);
    return 1;
fail:
    return 0;
}

uint8_t otic_unpack_close(otic_unpack_t* oticUnpack)
{
    otic_unpack_flush(oticUnpack);
    ZSTD_freeDCtx(oticUnpack->dCtx);
    size_t counter;
    for (counter = 0; counter < oticUnpack->cache_t.cache_allocated; counter++)
    {
        if (oticUnpack->cache_t.cache[counter]->last_value.string_value.size != 0){
//            free(oticUnpack->cache_t.cache[counter]->last_value.string_value.value);
        }
        free(oticUnpack->cache_t.cache[counter]);
    }
    free(oticUnpack->cache_t.cache);
    oticUnpack->cache_t.cache_allocated = oticUnpack->cache_t.totalEntries = oticUnpack->cache_t.allocationLeft = 0;
    oticUnpack->cache_t.currentEntry = 0;
    otic_base_setState(&oticUnpack->base, OTIC_STATE_CLOSED);
    return 1;
}