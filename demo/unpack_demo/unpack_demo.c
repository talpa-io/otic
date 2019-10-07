//
// Created by hp on 9/24/19.
//

#include <stdio.h>
#include "unpack/unpack.h"
#include "errHand/errHand.h"

FILE* srcFile = 0;
FILE* destFile = 0;

uint8_t fetcher(uint8_t* dest, size_t size)
{
    fread(dest, 1, size, srcFile);
    return 1;
}

uint8_t flusher(uint8_t* src, size_t size)
{
    fwrite(src, 1, size, destFile);
    return 1;
}


int main()
{
    srcFile = fopen("pack_demo.otic", "rb");
    destFile = fopen("dump.txt", "w");

    otic_unpack_t oticUnpack = {};
    if (!otic_unpack_init(&oticUnpack, fetcher, flusher)){
        printOticError(oticUnpack.base.error);
        return 1;
    }
    printOticError(oticUnpack.base.error);
    return 0;
}
