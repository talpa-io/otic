#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/unpack.h"
#include "utility/errHand.h"


static uint8_t fetcher(uint8_t* dest, size_t size, void* data)
{
    return fread(dest, 1, size, (FILE*)data) != 0;
}

static uint8_t flusher1(uint8_t* src, size_t size, void* data)
{
    return fwrite(src, 1, size, (FILE*)data) != 0;
}

static uint8_t flusher2(uint8_t* src, size_t size, void* data)
{
    return printf("%s", src) > 0;
}

/**
 * @brief To skip undefined channels' contents
 * @param pos The Position to skip to
 * @return 1/True on success, else 0/False on failure
 */
static uint8_t seeker(uint32_t pos, void* data)
{
    return !fseek((FILE*)data, pos, SEEK_CUR);
}

static int fpeek(FILE* stream)
{
    int c = fgetc(stream);
    ungetc(c, stream);
    return c;
}

int main()
{
    FILE* srcFile = fopen("pack_demo.otic", "rb");
    FILE* destFile = fopen("dump.tsv", "w");

    if (!srcFile || !destFile) {
        perror("fopen() failed!");
        return 1;
    }

    otic_unpack_t oticUnpack;
    if (!otic_unpack_init(&oticUnpack, fetcher, srcFile, seeker, srcFile))
        goto fail;

    otic_unpack_channel_t* channel1 = otic_unpack_defineChannel(&oticUnpack, 1, flusher1, destFile);
    if (!channel1)
        goto fail;
    const char* toFetch[] = {"sensor1", "sensor2"};
    otic_unpack_channel_toFetch(channel1, toFetch, 2);
    otic_unpack_channel_t* channel2 = otic_unpack_defineChannel(&oticUnpack, 2, flusher2, srcFile);
    if (!channel2)
        goto fail;

    while (fpeek(srcFile) != EOF)
        otic_unpack_parse(&oticUnpack);

    otic_unpack_close(&oticUnpack);
    fclose(destFile);
    fclose(srcFile);

    if (oticUnpack.state == OTIC_STATE_ON_ERROR)
        goto fail;
    return 0;

fail:
    printOticError(oticUnpack.error);
    return 1;
}
