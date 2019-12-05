#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <core/base.h>
#include "core/unpack.h"
#include "utility/errHand.h"


static uint8_t fetcher(uint8_t* dest, size_t size, void* data)
{
    return fread(dest, 1, size, (FILE*)data) != 0;
}

static uint8_t flusher1(double ts, const char* sensorName, const char* sensorUnit, const oval_t* value, void* data)
{
    switch (otic_oval_getType(value))
    {
        case OTIC_TYPE_NULL:
            fprintf((FILE*)data, "%lf\t%s\t%s\n", ts, sensorName, sensorUnit);
            break;
        case OTIC_TYPE_INT32_POS:
            fprintf((FILE*)data, "%lf\t%s\t%s\t%u\n", ts, sensorName, sensorUnit, value->lval.value);
            break;
        case OTIC_TYPE_INT32_NEG:
            fprintf((FILE*)data, "%lf\t%s\t%s\t-%u\n", ts, sensorName, sensorUnit, value->lval.value);
            break;
        case OTIC_TYPE_DOUBLE:
            fprintf((FILE*)data, "%lf\t%s\t%s\t%lf\n", ts, sensorName, sensorUnit, value->dval);
            break;
        case OTIC_TYPE_STRING:
            fprintf((FILE*)data, "%lf\t%s\t%s\t%s\n", ts, sensorName, sensorUnit, value->sval.ptr);
            break;
        default:
            return 0;
    }
    return 1;
}

static uint8_t flusher2(double ts, const char* sensorName, const char* sensorUnit, const oval_t* value, void* data)
{
    printf("%lf\t%s\t%s", ts, sensorName, sensorUnit);
    switch (otic_oval_getType(value))
    {
        case OTIC_TYPE_NULL:
            printf("\n");
            break;
        case OTIC_TYPE_INT32_NEG:
            printf("\t-%u\n", value->lval.value);
            break;
        case OTIC_TYPE_INT32_POS:
            printf("\t%u\n", value->lval.value);
            break;
        case OTIC_TYPE_STRING:
            printf("\t%s\n", value->sval.ptr);
            break;
        case OTIC_TYPE_DOUBLE:
            printf("\t%lf\n", value->dval);
            break;
        default:
            return 0;
    }
    return 1;
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
    FILE* destFile = fopen("unpack_demo.tsv", "w");
    if (!srcFile || !destFile) {
        perror("fopen() failed!");
        return 1;
    }
    otic_unpack_t oticUnpack;
    if (!otic_unpack_init(&oticUnpack, fetcher, srcFile, seeker, srcFile))
        goto fail;

    oticUnpackChannel_t* channel1 = otic_unpack_defineChannel(&oticUnpack, 1, flusher1, destFile);
    if (!channel1)
        goto fail;
    const char* toFetch[] = {"sensor1", "sensor2"};
    otic_unpack_channel_toFetch(channel1, toFetch, 2);
    oticUnpackChannel_t* channel2 = otic_unpack_defineChannel(&oticUnpack, 2, flusher2, srcFile);
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
