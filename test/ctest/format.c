#include <string.h>
#include <unity.h>
#include <utility/format.h>
#include <assert.h>
#include "utility/format.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_format_init_close(void)
{
    format_t format;
    format_init(&format, '\t', 4);
    TEST_ASSERT(format.delimiter == '\t')
    TEST_ASSERT(format.columns.size == 4)
    TEST_ASSERT(format.columns.content != 0)
    TEST_ASSERT(format.columns.parsed == 0)

    format_close(&format);
}

static void test_format_parse_simple(void)
{
    format_t format;
    format_init(&format, '\t', 5);
    char buffer[64] = {};
    strcpy(buffer, "This\tis\ta\ttest");
    format_parse(&format, buffer);

    TEST_ASSERT_EQUAL_size_t(4, format.columns.parsed);
    TEST_ASSERT_EQUAL_STRING("This", format.columns.content[0]);
    TEST_ASSERT_EQUAL_STRING("is", format.columns.content[1]);
    TEST_ASSERT_EQUAL_STRING("a", format.columns.content[2]);
    TEST_ASSERT_EQUAL_STRING("test", format.columns.content[3]);

    format_close(&format);
}

static void test_format_parse(void)
{
    format_t format;
    format_init(&format, ',', 10);
    char buffer[64] = {};
    strcpy(buffer, "A,,Some,.Weird,text,.,,.,\n");
    format_parse(&format, buffer);

    TEST_ASSERT_EQUAL_size_t(9, format.columns.parsed);
    TEST_ASSERT_EQUAL_STRING("A", format.columns.content[0]);
    TEST_ASSERT_EQUAL_STRING(0, format.columns.content[1]);
    TEST_ASSERT_EQUAL_STRING("Some", format.columns.content[2]);
    TEST_ASSERT_EQUAL_STRING(".Weird", format.columns.content[3]);
    TEST_ASSERT_EQUAL_STRING("text", format.columns.content[4]);
    TEST_ASSERT_EQUAL_STRING(".", format.columns.content[5]);
    TEST_ASSERT_EQUAL_STRING(0, format.columns.content[6]);
    TEST_ASSERT_EQUAL_STRING(".", format.columns.content[7]);
    TEST_ASSERT(*format.columns.content[8] == '\0')
    format_close(&format);
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_format_init_close);
    RUN_TEST(test_format_parse_simple);
    RUN_TEST(test_format_parse);
    UNITY_END();
}


/*
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "utility/format.h"


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
}*/
