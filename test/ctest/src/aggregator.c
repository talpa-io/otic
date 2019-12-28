#include "otic_test.h"
#include "utility/aggregator.h"
#include <float.h>
#include <limits.h>

OTIC_TEST_CASE(aggregator, aggregator_min)
{
    otic_aggreg_t aggreg;
    otic_aggreg_init(&aggreg, OTIC_AGGREG_MIN);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_MIN);
    TEST_ASSERT(aggreg.value.type == OTIC_TYPE_DOUBLE);
    TEST_ASSERT(aggreg.value.dval == DBL_MAX);

    



    otic_aggreg_close(&aggreg);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_NULL);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.counter == 0);
    TEST_ASSERT(aggreg.insert == 0);
    TEST_ASSERT(aggreg.get == 0);
}

OTIC_TEST_CASE(aggregator, aggregator_max)
{
    otic_aggreg_t aggreg;
}
