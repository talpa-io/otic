#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/unpack.h"
#include "utility/errHand.h"


static uint8_t fetcher(uint8_t* dest, size_t size, void* data)
{
    return fread(dest, 1, size, (FILE*)data) != 0;
}

static uint8_t flusher1(double ts, const char* sensorName, const char* sensorUnit, const oval_t* value, void* data)
{
#define FPRINTER(mod, val) fprintf((FILE*)data, "%lf\t%s\t%s\t"#mod"mod\n", ts, sensorName, sensorUnit, val)
    switch (otic_oval_getType(value))
    {
        case OTIC_TYPE_NULL:
            fprintf((FILE*)data, "%lf\t%s\t%s\n", ts, sensorName, sensorUnit);
            break;
        case OTIC_TYPE_INT_POS:
            FPRINTER("%lu", value->val.lval);
            break;
        case OTIC_TYPE_INT_NEG:
            FPRINTER("-%lu", value->val.lval);
            break;
        case OTIC_TYPE_DOUBLE:
            FPRINTER("%lf", value->val.dval);
            break;
        case OTIC_TYPE_STRING:
            FPRINTER("%s", value->val.sval.ptr);
            break;
        case OTIC_TYPE_TRUE:
            FPRINTER("%s", "[bool: true]");
            break;
        case OTIC_TYPE_FALSE:
            FPRINTER("%s", "[bool: false]");
            break;
        case OTIC_TYPE_ARRAY:
            FPRINTER("%s", "Array");
            break;
        default:
            return 0;
    }
    return 1;
#undef FPRINTER
}

static uint8_t oval_printer(const oval_t* val, FILE* dest)
{
#define PRINTER(fmt, ...) fprintf(dest, #fmt, __VA_ARGS__)
    switch (otic_oval_getType(val))
    {
        case OTIC_TYPE_NULL:
            PRINTER(%s, "(null)");
            break;
        case OTIC_TYPE_TRUE:
            PRINTER(%s, "(true)");
            break;
        case OTIC_TYPE_FALSE:
            PRINTER(%s, "(false)");
            break;
        case OTIC_TYPE_INT_POS:
            PRINTER(%lu, val->val.lval);
            break;
        case OTIC_TYPE_INT_NEG:
            PRINTER(-%lu, val->val.lval);
            break;
        case OTIC_TYPE_STRING:
            PRINTER(%s, val->val.sval.ptr);
            break;
        case OTIC_TYPE_FLOAT:
            PRINTER(%f, val->val.dval);
            break;
        case OTIC_TYPE_DOUBLE:
            PRINTER(%lf, val->val.dval);
            break;
        case OTIC_TYPE_ARRAY:
            PRINTER(%c, '[');
            for (uint32_t counter = 0; counter < val->val.aval.size; ++counter) {
                oval_printer(&val->val.aval.elements[counter], dest);
                if (counter != val->val.aval.size - 1)
                    PRINTER(%s, ", ");
            }
            PRINTER(%c, ']');
            break;
        case OTIC_TYPE_OBJECT:
            break;
        default:
            return 0;
    }
    return 1;
#undef PRINTER
}

static uint8_t flusher2(double ts, const char* sensorName, const char* sensorUnit, const oval_t* value, void* data)
{
    printf("%lf\t%s\t%s\t", ts, sensorName, sensorUnit);
    if (!oval_printer(value, stdout))
        return 0;
    printf("\n");
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

int main(void)
{
    FILE* srcFile = fopen("pack_demo.otic", "rb"), *destFile = fopen("unpack_demo.tsv", "w");
    otic_unpack_t oticUnpack;
    if (!otic_unpack_init(&oticUnpack, fetcher, srcFile, seeker, srcFile))
        return 1;
    oticUnpackChannel_t* channel = otic_unpack_defineChannel(&oticUnpack, 1, flusher2, destFile);
    if (!channel)
        return 1;
    while(otic_unpack_generate(&oticUnpack));
    otic_unpack_close(&oticUnpack);
    return 0;
}
//

//int main(void)
//{
//    FILE* srcFile = fopen("pack_demo.otic", "rb"), *destFile = fopen("unpack_demo.tsv", "w");
//    otic_unpack_t oticUnpack;
//    if (!otic_unpack_init(&oticUnpack, fetcher, srcFile, seeker, srcFile))
//        return 1;
//    oticUnpackChannel_t *channel = otic_unpack_defineChannel(&oticUnpack, 1, flusher2, destFile);
//    if (!channel)
//        return 1;
//
//    while (otic_unpack_parse(&oticUnpack));
//
//    otic_unpack_close(&oticUnpack);
//    return 0;
//}
//
//int main()
//{
//    FILE* srcFile = fopen("pack_demo.otic", "rb");
//    FILE* destFile = fopen("unpack_demo.tsv", "w");
//    if (!srcFile || !destFile) {
//        perror("fopen() failed!");
//        return 1;
//    }
//    otic_unpack_t oticUnpack;
//    if (!otic_unpack_init(&oticUnpack, fetcher, srcFile, seeker, srcFile))
//        goto fail;
//
//    oticUnpackChannel_t* channel1 = otic_unpack_defineChannel(&oticUnpack, 1, flusher2, destFile);
//    if (!channel1)
//        goto fail;
//    const char* toFetch[] = {"sensor1", "sensor2"};
//    otic_unpack_channel_toFetch(channel1, toFetch, 2);
//    oticUnpackChannel_t* channel2 = otic_unpack_defineChannel(&oticUnpack, 2, flusher2, srcFile);
//    if (!channel2)
//        goto fail;
//
//    //while (fpeek(srcFile) != EOF)
//    //    otic_unpack_parse(&oticUnpack);
//
//    while (otic_unpack_parse(&oticUnpack));
//
//    otic_unpack_close(&oticUnpack);
//    fclose(destFile);
//    fclose(srcFile);
//
//    if (oticUnpack.state == OTIC_STATE_ON_ERROR)
//        goto fail;
//    return 0;
//
//fail:
//    printOticError(oticUnpack.error);
//    return 1;
//}
//