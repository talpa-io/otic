#include <otic_test.h>
#include <aggregator.h>


OTIC_TEST_CASE(aggregator, init_close)
{
    otic_aggreg_t aggreg;
    otic_aggreg_init(&aggreg, OTIC_AGGREG_MIN);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
}

