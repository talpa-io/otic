//
// Created by hp on 9/20/19.
//

#ifndef OTIC_UNPACK_H
#define OTIC_UNPACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zstd.h>
#include "../core/core.h"

#define OTIC_UNPACK_OUT_SIZE 1024
#define OTIC_UNPACK_OUTPUT_LIMIT 512
#define OTIC_UNPACK_RESULT_OUTSIZE 2056
#if OTIC_UNPACK_RESULT_OUTSIZE < (OTIC_UNPACK_OUTPUT_LIMIT * 2)
#error OTIC unpack requires the Output buffer to be twice as big as the threshold limit
#endif
#define OTIC_UNPACK_CACHE_ALLOCATION_RESERVE_SIZE 8

typedef struct
{
    uint32_t index;
    char* name;
    char* unit;
    union{
        uint32_t int_value;
        double double_value;
        struct
        {
            char* value;
            size_t size;
        } string_value;
    } last_value;
    otic_types_e type;

} otic_unpack_entry_t;

typedef struct
{
    otic_base_t base;
    ZSTD_DCtx* dCtx;
    struct {
        otic_unpack_entry_t** cache;
        size_t totalEntries;
        uint8_t allocationLeft;
        size_t cache_allocated;
        otic_unpack_entry_t* currentEntry;
    } cache_t;
    struct{
        char content[OTIC_UNPACK_RESULT_OUTSIZE];
        char* top;
        char* limit;
    } result;
    uint8_t out[OTIC_UNPACK_OUT_SIZE];
    uint8_t(*fetcher)(uint8_t*, size_t);
    uint8_t(*flusher)(uint8_t*, size_t);
    size_t blockSize;
    double doubleTs;
} otic_unpack_t;

uint8_t otic_unpack_init(otic_unpack_t* oticUnpack, uint8_t(*fetcher)(uint8_t*, size_t), uint8_t(*flusher)(uint8_t*, size_t));
uint8_t otic_unpack_parseBlock(otic_unpack_t*);
void otic_unpack_flush(otic_unpack_t* oticUnpack);
uint8_t otic_unpack_close(otic_unpack_t* oticUnpack);


#ifdef __cplusplus
}
#endif


#endif //OTIC_UNPACK_H