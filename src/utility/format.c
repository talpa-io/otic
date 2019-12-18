#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utility/format.h"


uint8_t format_init(format_t *restrict format, char delimiter, size_t numb_columns)
{
    if (!format || !(format->columns.content = malloc(numb_columns * sizeof(char*))))
        return 0;
    format->delimiter = delimiter;
    format->columns.size = numb_columns;
    format->columns.parsed = 0;
    return 1;
}

char* format_parse(format_t *restrict format, char* line)
{
    char** ptr = format->columns.content;
    format->columns.parsed = 1;
    *ptr = line;
    while (*line)
    {
        if (*line == '\n') {
            *line = 0;
            break;
        }
        if (*line == format->delimiter) {
            *line = 0;
            *++ptr = (*(line + 1) == format->delimiter || *(line + 1) == '\n') ? 0 : line + 1;
            ++format->columns.parsed;
        }
        ++line;
    }
    return line + 1;
}

size_t format_write(format_t *restrict format, char* dest)
{
    static size_t counter = 0;
    for (counter = 0; counter < format->columns.parsed - 1; counter++) {
        dest += !format->columns.content[counter]? sprintf(dest,"%c", format->delimiter) : sprintf(dest, "%s%c", format->columns.content[counter], format->delimiter);
    }
    if (format->columns.content[counter])
        sprintf(dest, "%s", format->columns.content[counter]);
    return 1;
}

void format_reset(format_t* restrict format)
{
    size_t counter;
    for (counter = 0; counter < format->columns.size; ++counter)
        format->columns.content[counter] = 0;
}

void format_close(format_t *restrict format)
{
    free(format->columns.content);
}

uint8_t format_chunker_init(format_chunker_t* formatChunker, char delimiter, size_t columnSize)
{
    if (!formatChunker)
        return 0;
    if (!format_init(&formatChunker->format, delimiter, columnSize))
        return 0;
    return 1;
}

void format_chunker_set(format_chunker_t* formatChunker, char* chunk, size_t size)
{
    formatChunker->ptr_current = formatChunker->ptr_start = chunk;
    formatChunker->size = size;
}

char* format_chunker_parse(format_chunker_t* formatChunker)
{
    return format_parse(&formatChunker->format, formatChunker->ptr_current);
}

void format_chunker_close(format_chunker_t* formatChunker)
{
    format_close(&formatChunker->format);
}