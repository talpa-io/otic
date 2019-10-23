//
// Created by talpaadmin on 02.10.19.
//

#ifndef OTIC_FORMAT_H
#define OTIC_FORMAT_H


#include <stdint.h>
#include <stddef.h>


typedef struct
{
    char delimiter;
    struct
    {
        char** content;
        size_t size;
        size_t parsed;
    } columns;

} format_t;

uint8_t     format_init(format_t *restrict format, char delimiter, size_t numb_columns);
char*       format_parse(format_t *restrict format, char* line, uint8_t newLineBreak);
size_t      format_write(format_t *restrict format, char* dest);
void        format_reset(format_t *restrict format);
void        format_close(format_t *restrict format);


typedef struct
{
    format_t format;
    char* ptr_start;
    char* ptr_current;
    size_t size;
} format_chunker_t;

uint8_t     format_chunker_init(format_chunker_t* formatChunker, char delimiter, size_t columnSize);
void        format_chunker_set(format_chunker_t* formatChunker, char* chunk, size_t size);
char*       format_chunker_parse(format_chunker_t* formatChunker);
void        format_chunker_close(format_chunker_t* formatChunker);


#endif //OTIC_FORMAT_H