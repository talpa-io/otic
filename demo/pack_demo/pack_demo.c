//
// Created by hp on 9/22/19.
//

#include <stdio.h>
#include "pack/pack.h"
#include "errHand/errHand.h"

FILE* fileOut = 0;

static uint8_t flusher(uint8_t* src, size_t size)
{
    fwrite(src, 1, size, fileOut);
    return 1;
}


int main()
{
//    fileOut = fopen("pack_demo.otic", "wb");
//    if (!fileOut)
//        return 1;
//    otic_pack_t oticPack = {};
//    if (!otic_pack_init(&oticPack, flusher, "Some Metadata")){
//        printOticError(otic_base_getError(&oticPack.base));
//        return 1;
//    }
//    otic_pack_inject_i(&oticPack, 12345, "sensor1", "unit1", 1234);
//    otic_pack_inject_i(&oticPack, 12345, "sensor2", "unit1", 1234);
//    otic_pack_inject_i(&oticPack, 12345, "sensor3", "unit1", 1234);
//    otic_pack_inject_i(&oticPack, 12345, "sensor4", "unit1", 1234);
//    otic_pack_inject_i(&oticPack, 12345, "sensor5", "unit1", 1234);
//    otic_pack_inject_i(&oticPack, 12345, "sensor5", "unit1", 1234);
//
//    otic_pack_close(&oticPack);
    return 0;
}