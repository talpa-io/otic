#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unpack/unpack.h>
#include "pack/pack.h"
#include "unpack/unpack.h"
#include "errHand/errHand.h"


static FILE* file;
static otic_pack_t oticPack;
static otic_unpack_t oticUnpack;

static uint8_t buffer[1024];

static uint8_t pack_flusher(uint8_t* cont, size_t size)
{
    assert(size < 1024);
    static uint8_t* top = buffer;
    memcpy(top, cont, size);
    top += size;
    return 1;
}

static uint8_t unpack_flusher(uint8_t* cont, size_t size)
{
    printf("%s\n", cont);
    return 1;
}

static uint8_t unpack_fetcher(uint8_t* cont, size_t size)
{
    assert(size < 1024);
    static uint8_t* top = buffer;
    memcpy(cont, top, size);
    top += size;
    return 1;
}


int main(void)
{
    if (!otic_pack_init(&oticPack, pack_flusher, "Some metaData"))
        goto fail_pack;
    otic_pack_inject_i(&oticPack, 1.2, "sensorName", "sensorUnit", 9876);
    otic_pack_inject_i(&oticPack, 1.2, "sensorName2", "sensorUnit2", 9876);
    otic_pack_inject_i(&oticPack, 12345, "sensor1", "unit1", 9875);
    otic_pack_inject_i(&oticPack, 12345, "sensor2", "unit1", 9876);
    otic_pack_inject_d(&oticPack, 12346, "sensor9", "unit1", 9876);
    otic_pack_inject_d(&oticPack, 12346, "sensor9", "unit1", 9876);
    otic_pack_inject_d(&oticPack, 12346, "sensor9", "unit1", 9875.43);
    otic_pack_inject_i(&oticPack, 12345, "sensor4", "unit1", 9877);
    otic_pack_inject_i(&oticPack, 12345, "sensor5", "unit1", -9877);
    otic_pack_inject_i(&oticPack, 12345, "sensor5", "unit1", -9877);
    otic_pack_inject_i(&oticPack, 12345, "sensor5", "unit1", -9875);
    otic_pack_inject_n(&oticPack, 23434, "sensor6", "unit3");
    otic_pack_inject_n(&oticPack, 23434, "sensor6", "unit3");
    otic_pack_inject_s(&oticPack, 23434, "sensor7", "unit3", "Hallo World");
    otic_pack_inject_s(&oticPack, 23434, "sensor7", "unit3", "Hallo World");
    otic_pack_inject_s(&oticPack, 23434, "sensor7", "unit3", "Hey man!");
    otic_pack_flush(&oticPack);
    if (!otic_unpack_init(&oticUnpack, unpack_fetcher, unpack_flusher))
        goto fail_unpack;
//    otic_unpack_flush(&oticUnpack);
//    printf("%s\n", oticUnpack.result.content);
    if (!otic_pack_close(&oticPack))
        goto fail_pack;
    if (!otic_unpack_close(&oticUnpack))
        goto fail_pack;

    return EXIT_SUCCESS;
fail_pack:
    fprintf(stderr, "Pack_error->");
    printOticError(otic_base_getError(&oticPack.base));
    return EXIT_FAILURE;
fail_unpack:
    fprintf(stderr, "Unpack_error->");
    printOticError(otic_base_getError(&oticUnpack.base));
    return EXIT_FAILURE;
}