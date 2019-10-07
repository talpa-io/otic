//
// Created by talpaadmin on 07.10.19.
//

#include <stdio.h>
#include <stdlib.h>
#include "pack/pack.h"


uint8_t flusher(uint8_t* content, size_t size)
{
    printf("%s\n", content);
    return 1;
}

int main()
{
    otic_pack_base_t oticPackBase;

    otic_pack_base_init(&oticPackBase);
    channel_t* channel;
    if (!(channel = otic_pack_base_defineChannel(&oticPackBase, OTIC_PACK_CHANNEL_SENSOR, 1, flusher)))
        return 1;

    return EXIT_SUCCESS;
}