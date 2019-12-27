#include <string.h>
#include <utility/format.h>
#include <assert.h>
#include "utility/format.h"
#include "otic_test.h"

OTIC_TEST_CASE(format_test, init_close)
{
    format_t format;
    format_init(&format, '\t', 4);
    TEST_ASSERT(format.delimiter == '\t')
    TEST_ASSERT(format.columns.size == 4)
    TEST_ASSERT(format.columns.content != 0)
    TEST_ASSERT(format.columns.parsed == 0)

    format_close(&format);
}

OTIC_TEST_CASE(format_test, parse_simple)
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

// TODO: nullptr at start if nothing is given
OTIC_TEST_CASE(format_test, parse)
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
//    TEST_ASSERT(*format.columns.content[8] == '\0')
    format_close(&format);
}

OTIC_TEST_CASE(format_test, parse_complex)
{
    format_t format;
    format_init(&format, '|', 8);
    char buffer[64] = {};
    strcpy(buffer, "||||||hallo world|");
    format_parse(&format, buffer);
    TEST_ASSERT(*format.columns.content[0] == '\0')
    TEST_ASSERT(format.columns.content[1] == 0)
    TEST_ASSERT(format.columns.content[2] == 0)
    TEST_ASSERT(format.columns.content[3] == 0)
    TEST_ASSERT(format.columns.content[4] == 0)
    TEST_ASSERT(format.columns.content[5] == 0)
    TEST_ASSERT_EQUAL_STRING("hallo world", format.columns.content[6]);

    format_close(&format);
}

