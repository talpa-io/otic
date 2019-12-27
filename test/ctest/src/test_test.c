#include <stdio.h>
#include "otic_test.h"

OTIC_TEST_CASE(test_test, test1)
{
    printf("%s\n", __PRETTY_FUNCTION__);
}

OTIC_TEST_CASE(test_test, test2)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    TEST_ASSERT(0);
}
