#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <otic.h>
#include "otic_test.h"
#include "core/pack.h"

#define TEST_BUFFERSIZE 512
char buffer[TEST_BUFFERSIZE];

static size_t getMemomryFriendlyApprox(size_t value)
{
    size_t retval = 1;
    while (retval < value)
        retval <<= 1u;
    return retval;
}

uint8_t flusher(uint8_t* content, size_t size, void* data)
{
    UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT64(TEST_BUFFERSIZE, size, __LINE__, "Input too long");
    memcpy(buffer, content, size);
    return 1;
}

otic_pack_t packer;
size_t bucketSize;

void setUp()
{
    bucketSize = getMemomryFriendlyApprox(15000);
    otic_pack_init(&packer, 0x00, flusher, buffer);
}

void tearDown()
{
    otic_pack_close(&packer);
}



OTIC_TEST_CASE(otic_pack, init_close)
{
    TEST_ASSERT_EQUAL_HEX8(OTIC_STATE_OPENED, packer.state);
    TEST_ASSERT_EQUAL_HEX8(OTIC_ERROR_NONE, packer.error);
    TEST_ASSERT(packer.flusher);
    TEST_ASSERT(!packer.channels);
    TEST_ASSERT(packer.data);
    TEST_ASSERT(packer.data);

    TEST_ASSERT_EQUAL_STRING_LEN(OTIC_MAGIC, buffer, 4);
}

OTIC_TEST_CASE(otic_pack, defineChannel)
{
    const size_t bufferSize = getMemomryFriendlyApprox(15000);
    otic_pack_channel_t* channel = otic_pack_defineChannel(&packer, OTIC_CHANNEL_TYPE_SENSOR, 0x08, 0x0, bufferSize);
    TEST_ASSERT_NOT_NULL(channel);
    TEST_ASSERT_NOT_NULL(channel->ztd_out);
    TEST_ASSERT_NOT_NULL(channel->cCtx);
    TEST_ASSERT_EQUAL_HEX8(channel->info.channelId, 0x08);
    TEST_ASSERT_EQUAL_UINT8(channel->info.channelType, OTIC_CHANNEL_TYPE_SENSOR);
    TEST_ASSERT_EQUAL_PTR(channel->info.parent, &packer);
    TEST_ASSERT(channel->timeInterval.time_start == TS_NULL);
    TEST_ASSERT(channel->totalEntries == 0);
#ifdef OTIC_STATS
    TEST_ASSERT_EQUAL(channel->stats.time_shifts, 0);
    TEST_ASSERT_EQUAL(channel->stats.cols_assigned, 0);
    TEST_ASSERT_EQUAL(channel->stats.time_sets, 0);
    TEST_ASSERT_EQUAL(channel->stats.type_null, 0);
    TEST_ASSERT_EQUAL(channel->stats.type_object, 0);
    TEST_ASSERT_EQUAL(channel->stats.type_array, 0);
    TEST_ASSERT_EQUAL(channel->stats.type_double, 0);
    TEST_ASSERT_EQUAL(channel->stats.time_shifts, 0);
    TEST_ASSERT_EQUAL(channel->stats.type_string, 0);
    TEST_ASSERT_EQUAL(channel->stats.type_integer, 0);
    TEST_ASSERT_EQUAL(channel->stats.time_sets, 0);
    TEST_ASSERT_EQUAL(channel->stats.type_bool, 0);
    TEST_ASSERT_EQUAL(channel->stats.blocksWritten, 0);
    TEST_ASSERT_EQUAL(channel->stats.type_unmodified, 0);
#endif
    TEST_ASSERT_EQUAL_PTR(channel->base.cache, channel->base.top);
    TEST_ASSERT_NOT_NULL(channel->cCtx);
    TEST_ASSERT_EQUAL_PTR(channel->threshold, channel->base.top + bufferSize - OTIC_PACK_CACHE_TOP_LIMIT);
}

OTIC_TEST_CASE(otic_pack, define_channel_sanitize)
{
}



OTIC_TEST_CASE(otic_pack, close_channel)
{
//    const size_t bucketSize = getMemomryFriendlyApprox(15000);
//    otic_pack_channel_t* channel = otic_pack_defineChannel(&packer, OTIC_CHANNEL_TYPE_SENSOR, 0x01, 0x00, bucketSize);
}

#undef TEST_BUFFERSIZE