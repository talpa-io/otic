//
// Created by talpaadmin on 02.10.19.
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "format/format.h"

static char buffer[24] = {};
static char result[24] = {};
int main()
{
    format_t format;
    assert(format_init(&format, ':', 5));
    strcpy(buffer, "This:is:a:test");
    size_t len = strlen(buffer);
    assert(format_parse(&format, buffer, 0) == 4);
    format_write(&format, result);
    assert(format_write(&format, result) == len);
    assert(strcmp(buffer, result) != 0);
    format_close(&format);

    return 0;
}