//
// Created by talpaadmin on 02.10.19.
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "format/format.h"


static void test_format_init(void)
{
    format_t format;
    assert(!format_init(0, '\t', 5));
    assert(format_init(&format, '\t', 5));
    format_close(&format);
}

static void test_format_parseSimple(void)
{
    format_t format;
    format_init(&format, ':', 5);
    char buffer[24] = {}, line[24], result[24] = {};
    strcpy(buffer, "This:is:a:simple:test");
    strcpy(line, "This:is:a:simple:test");
    format_parse(&format, buffer);
//    assert(strcmp(format.columns.content[0], "This") == 0);
//    assert(strcmp(format.columns.content[1], "is") == 0);
//    assert(strcmp(format.columns.content[2], "a") == 0);
//    assert(strcmp(format.columns.content[3], "simple") == 0);
//    assert(strcmp(format.columns.content[4], "test") == 0);
//    format_write(&format, result);
    format_close(&format);
}

static void test_format_parseComplex(void)
{
    format_t format;
    format_init(&format, '\t', 4);
    char buffer[24] = {}, line[24], result[24] = {};
    strcpy(buffer, "This\t\tTest");
    strcpy(line, "This\t\tTest");
    format_parse(&format, buffer);
    assert(strcmp(format.columns.content[0], "This") == 0);
    assert(format.columns.content[1] == 0);
    assert(strcmp(format.columns.content[2], "Test") == 0);
    format_write(&format, result);
    assert(strcmp(line, result) == 0);
    format_close(&format);
}

static void test_format_parseComplexer(void)
{
    format_t format;
    format_init(&format, ',', 5);
    char buffer[24] = {}, line[24], result[24] = {};
    strcpy(buffer, "This,,,,value");
    strcpy(line, "This,,,,value");
    format_parse(&format, buffer);
    assert(strcmp(format.columns.content[0], "This") == 0);
    assert(format.columns.content[1] == 0);
    assert(format.columns.content[2] == 0);
    assert(format.columns.content[3] == 0);
    assert(strcmp(format.columns.content[4], "value") == 0);
    format_write(&format, result);
    assert(strcmp(line, result) == 0);
    format_close(&format);
}


int main()
{
    test_format_init();
    test_format_parseSimple();
//    test_format_parseComplex();
//    test_format_parseComplexer();
//
    return 0;
}