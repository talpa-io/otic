//
// Created by talpaadmin on 02.10.19.
//

#include <stdio.h>
#include "format/format.h"
#include <string.h>
#include <format/format.h>

static char buffer[24];

int main()
{
    format_t format;
    if (!format_init(&format, ':', 5))
        return 1;

    strcpy(buffer, "This:is:a::test");
    long ret = format_parse(&format, buffer, 0);
    for (long counter = 0; counter < ret; counter++)
    {
        printf("%s\n", format.columns.content[counter]);
    }

//    format_write(&format);
    format_close(&format);

    return 0;
}