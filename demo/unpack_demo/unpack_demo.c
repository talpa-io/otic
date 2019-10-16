//
// Created by hp on 9/24/19.
//

#include <stdio.h>
#include "unpack/unpack.h"
#include "errHand/errHand.h"

static FILE* srcFile = 0;
static FILE* destFile = 0;

static uint8_t fetcher(uint8_t* dest, size_t size)
{
    fread(dest, 1, size, srcFile);
    return 1;
}

static uint8_t flusher(uint8_t* src, size_t size)
{
    fwrite(src, 1, size, destFile);
    return 1;
}

static uint8_t flusher2(uint8_t* src, size_t size)
{
    printf("%s", src);
    return 1;
}

/**
 * @brief To skip undefined channels' contents
 * @param pos The Position to skip to
 * @return 1/True on success, else 0/False on failure
 */
static uint8_t seeker(uint32_t pos)
{
    return !fseek(srcFile, pos, SEEK_CUR);
}

static int fpeek(FILE* stream)
{
    if (fgetc(stream) == -1)
        return 1;
    fseek(stream, -1, SEEK_CUR);
    return 0;
}

int main()
{
    srcFile = fopen("pack_demo.otic", "rb");
    destFile = fopen("dump.tsv", "w");

    otic_unpack_t oticUnpack;
    if (!otic_unpack_init(&oticUnpack, fetcher, seeker))
        goto fail;
    otic_unpack_defineChannel(&oticUnpack, 1, flusher);
    otic_unpack_defineChannel(&oticUnpack, 2, flusher2);
    while (!fpeek(srcFile))
    {
        otic_unpack_parse(&oticUnpack);
    }
    otic_unpack_close(&oticUnpack);

    if (oticUnpack.state == OTIC_STATE_ON_ERROR)
        goto fail;
    return 0;
fail:
    printOticError(oticUnpack.error);
    return 1;
}
