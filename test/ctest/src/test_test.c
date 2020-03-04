/***
 * @file test_test.c
 * @brief Test the testing library
 */

#include <stdio.h>
#include "otic_test.h"

void setUp()
{

}

void tearDown()
{

}

OTIC_TEST_CASE(test_test, test1)
{
    TEST_ASSERT(1);
    TEST_ASSERT_FALSE(0)
    TEST_ASSERT_EQUAL(2342, 2342);
}

OTIC_TEST_CASE(test_test, test2)
{
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x10, 0x10, "Some Message");
    TEST_ASSERT_EQUAL_MEMORY("Some Test", "Some Test", 10);
    TEST_ASSERT_DOUBLE_IS_NOT_INF(12342.0F);
    TEST_ASSERT_DOUBLE_WITHIN(20, 10, 15);
    TEST_ASSERT_EQUAL_STRING_LEN("Another string", "Another string", 14);
    char a[2];
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_EQUAL(1, 2);
}

OTIC_TEST_CASE(test_test, test3)
{
    TEST_ASSERT_EQUAL_HEX8(0x10, 0x10);
    TEST_ASSERT_EQUAL_UINT16(1234, 1234);
    TEST_ASSERT_EQUAL_UINT32(12344556, 12344556);
    TEST_ASSERT_EQUAL_UINT64(1322334545, 1322334545);
    TEST_ASSERT_NOT_EQUAL(2324, 2345);
}