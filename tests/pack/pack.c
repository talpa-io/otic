//
// Created by hp on 9/25/19.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <zstd.h>
#include <limits.h>
#include "pack/pack.h"
#include "unpack/unpack.h"

static char buffer[UINT8_MAX * 2];

uint8_t flusher(uint8_t* a, size_t size)
{
    ZSTD_decompress(buffer, sizeof(buffer), a, size);
    return 1;
}

static void test_pack_init_close(void)
{
//    otic_pack_t oticPack;
//    assert(!otic_pack_init(&oticPack, 0, "I am supposed to be some metadata"));
//    assert(oticPack.base.error == OTIC_ERROR_INVALID_POINTER);
//    assert(otic_pack_init(&oticPack, flusher, "Back again! What does meta even mean?"));
//    assert(otic_pack_flush(&oticPack));
//    char* top = buffer;
//    assert(strcmp("otic", top) == 0);
//    top += 5;
//    assert(*top++ == OTIC_TYPE_FILE_VERSION);
//    assert(*top++ == OTIC_VERSION_MAJOR);
//    assert(*top++ == OTIC_TYPE_METADATA);
//    uint32_t value;
//    top += leb128_decode_unsigned((uint8_t*)top, &value);
//    assert(value == strlen("Back again! What does meta even mean?"));
//    assert(strcmp(top, "Back again! What does meta even mean?") == 0);
//    assert(otic_pack_close(&oticPack));
}

static void test_pack_writes(void)
{
//    otic_pack_t oticPack;
//    otic_pack_init(&oticPack, flusher, "Hopefully this code doesn't pull a homer");
//    assert(otic_pack_flush(&oticPack));
//    memset(buffer, 0, sizeof(buffer));
//    assert(otic_pack_inject_i(&oticPack, 12345, "sensor1", "unit1", 9876));
//    assert(otic_pack_inject_i(&oticPack, 12346, "sensor1", "unit1", 9876));
//    assert(otic_pack_inject_i(&oticPack, 12346, "sensor1", "unit1", 9876));
//    assert(otic_pack_flush(&oticPack));
//    otic_pack_close(&oticPack);
}

typedef struct
{
    int a;
    double b;
} myStruct_t;


void t(myStruct_t* myStruct)
{

}

int main(void)
{
    test_pack_init_close();
    test_pack_writes();

    return 0;
}