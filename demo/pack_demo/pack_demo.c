//
// Created by hp on 9/22/19.
//

#include <stdio.h>
#include "pack/pack.h"
#include "errHand/errHand.h"

/**
 * @file pack_demo.c
 * @brief Simple pack demo
 *
 * This file's purpose is to present the developer how to pack simple data
 */


static FILE* fileOut = 0;
static uint8_t flusher(uint8_t* src, size_t size)
{
    fwrite(src, 1, size, fileOut);
    return 1;
}

int main()
{
    fileOut = fopen("pack_demo.otic", "wb");
    if (!fileOut)
        return 1;
    otic_pack_t oticPack;
    if (!otic_pack_init(&oticPack, flusher))
        goto fail;
    otic_pack_channel_t* channel1 = otic_pack_defineChannel(&oticPack, OTIC_CHANNEL_TYPE_SENSOR, 1, 0x00);
    if (!channel1)
        goto fail;
    otic_pack_channel_t* channel2 = otic_pack_defineChannel(&oticPack, OTIC_CHANNEL_TYPE_BINARY, 2, 0x00);
    if (!channel2)
        goto fail;

    otic_pack_channel_inject_i(channel1, 1234.4, "sensor1", "sensorUnit1", 1232434);
    otic_pack_channel_inject_d(channel1, 1234.5, "sensor2", "sensorUnit2", 3.1417);
    otic_pack_channel_inject_i_neg(channel1, 1234.5, "sensor1", "sensorUnit1", 54);
    otic_pack_channel_inject_s(channel1, 12323, "sensor3", "sensorUnit3", "Some string");
    otic_pack_channel_inject_n(channel1, 12456, "sensor1", "sensorUnit1");

    otic_pack_channel_inject_i(channel2, 1234.4, "sensor1", "sensorUnit1", 1232434);
    otic_pack_channel_inject_d(channel2, 1234.5, "sensor2", "sensorUnit2", 3.1417);
    otic_pack_channel_inject_i_neg(channel2, 1234.5, "sensor1", "sensorUnit1", 54);
    otic_pack_channel_inject_s(channel2, 12323, "sensor3", "sensorUnit3", "Some string");
    otic_pack_channel_inject_n(channel2, 12456, "sensor1", "sensorUnit1");

    otic_pack_close(&oticPack);
    return 0;
fail:
    printOticError(oticPack.error);
    return 1;
}