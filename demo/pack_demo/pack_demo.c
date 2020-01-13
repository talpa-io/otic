/**
 * @file pack_demo.c
 * @brief Simple pack demo
 *
 * This file's purpose is to present the developer how the basic functions of the pack are used
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "core/pack.h"
#include "utility/errHand.h"

static uint8_t flusher(uint8_t* src, size_t size, void* file)
{
    return fwrite(src, 1, size, (FILE*)file) != 0;
}

int main()
{
    FILE* fileOut = 0;
    fileOut = fopen("pack_demo.otic", "wb");

    if (!fileOut)
        goto system_fail;
    otic_pack_t oticPack;
    if (!otic_pack_init(&oticPack, flusher, fileOut))
        goto fail;
    otic_pack_channel_t* channel1 = otic_pack_defineChannel(&oticPack, OTIC_CHANNEL_TYPE_SENSOR, 1, 0x00);
    if (!channel1)
        goto fail;
    otic_pack_channel_t* channel2 = otic_pack_defineChannel(&oticPack, OTIC_CHANNEL_TYPE_SENSOR, 2, 0x00);
    if (!channel2)
        goto fail;

    otic_pack_channel_inject_i(channel1, 1223, "sensor1", "unit1", 12);
    otic_pack_channel_inject_i(channel1, 1234.4, "sensor1", "sensorUnit1", 1232434);
    otic_pack_channel_inject_d(channel1, 1234.5, "sensor2", "sensorUnit2", 3.1417);
    otic_pack_channel_inject_i_neg(channel1, 1234.5, "sensor1", "sensorUnit1", 54);
    otic_pack_channel_inject_s(channel1, 12323, "sensor1", "sensorUnit1", "Some string");
    otic_pack_channel_inject_s(channel1, 12323, "sensor3", "sensorUnit3", "Some string");
    otic_pack_channel_inject_s(channel1, 12323, "sensor3", "sensorUnit3", "Some other string");
    otic_pack_channel_inject_s(channel1, 12323, "sensor3", "sensorUnit3", "sd");
    otic_pack_channel_inject_n(channel1, 12456, "sensor1", "sensorUnit1");

    otic_pack_channel_inject_i(channel2, 1234.4, "sensor1", "sensorUnit1", 1232434);
    otic_pack_channel_inject_d(channel2, 1234.5, "sensor1", "sensorUnit1", 3.1417);
    otic_pack_channel_inject_i_neg(channel2, 1234.5, "sensor1", "sensorUnit1", 54);
    otic_pack_channel_inject_n(channel2, 12456, "sensor1", "sensorUnit1");
    otic_pack_channel_inject_s(channel2, 12456, "sensor1", "sensorUnit1", "Some string");
    otic_pack_channel_inject_s(channel2, 12456, "sensor1", "sensorUnit1", "Some string1");
    otic_pack_channel_inject_s(channel2, 12456, "sensor1", "sensorUnit1", "Some string");

    // Not needed. This feature was added to allow early file closes,
    // as otic_pack_close destroys every created channels, that isn't closed!
    //otic_pack_channel_close(channel1);

    otic_pack_close(&oticPack);
    fclose(fileOut);
    return 0;

fail:
    printf("%s\n", otic_strError(oticPack.error));
    return 1;

system_fail:
    printf("%s\n", strerror(errno));
    return errno;
}