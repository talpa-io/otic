//
// Created by talpaadmin on 02.10.19.
//

#include <stdlib.h>
#include <stdio.h>
#include "format.h"


uint8_t format_init(format_t* format, char delimiter, size_t numb_columns)
{
    if (!(format->columns.content = malloc(numb_columns * sizeof(char*))))
        return 0;
    format->delimiter = delimiter;
    format->columns.size = numb_columns;
    return 1;
}

ptrdiff_t format_parse(format_t* format, char* line, uint8_t newLineBreak)
{
    char** ptr = format->columns.content;
    *ptr = line;
    while (*line)
    {
        if (*line == format->delimiter)
        {
            *line = 0;
            *++ptr = line + 1;
        }
        line++;
    }
    return ptr - format->columns.content + 1;
}

char* escape(char* value)
{
    return 0;
}

size_t format_write(format_t* format, char* dump)
{
    static size_t counter = 0;
    size_t written = 0;
    for (counter = 0; counter < format->columns.size - 1; counter++)
        written += sprintf(dump + written, "%s%c", format->columns.content[counter] ? format->columns.content[counter] : "", format->delimiter);
    written += sprintf(dump + written, "%s\n", format->columns.content[counter] ? format->columns.content[counter] : "");
    return written;
}

void format_close(format_t* format)
{
    free(format->columns.content);
}