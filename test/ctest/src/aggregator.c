#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stddef.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include "otic_test.h"
#include "utility/aggregator.h"

// These tests would have looked nicer had they been written in C++.

static oval_t aggreg_min_lambda(oval_t* randContainer, size_t size)
{
    double min = DBL_MAX;
    for (typeof(size) counter = 0; counter < size; counter++)
    {
        if (randContainer[counter].dval < min)
            min = randContainer[counter].dval;
    }
    return (oval_t){.type = (min == DBL_MAX ? OTIC_TYPE_NULL: OTIC_TYPE_DOUBLE), .dval = min}; 
}

static oval_t aggreg_max_lambda(oval_t* randContainer, size_t size)
{
    double max = DBL_MIN;
    for (typeof(size) counter = 0; counter < size; ++counter)
        if (randContainer[counter].dval > max)
            max = randContainer[counter].dval;
    return (oval_t){.type = (max == DBL_MIN ? OTIC_TYPE_NULL: OTIC_TYPE_DOUBLE), .dval = max};
}

static oval_t aggreg_avg_lambda(oval_t* randContainer, size_t size)
{
    double avg = 0;
    for (typeof(size) counter = 0; counter < size; ++counter)
        avg += randContainer[counter].dval;
    return (oval_t){.type = OTIC_TYPE_DOUBLE, .dval = avg / size};
}

static oval_t aggreg_sum_lambda(oval_t* randContainer, size_t size)
{
    double sum;
    for (typeof(size) counter = 0;  counter < size; ++counter)
        sum += randContainer[counter].dval;
    return (oval_t){.type = OTIC_TYPE_DOUBLE, .dval = sum};
}


OTIC_TEST_CASE(aggregator, aggregator_min)
{
    otic_aggreg_t aggreg;
    otic_aggreg_init(&aggreg, OTIC_AGGREG_MIN);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_MIN);
    TEST_ASSERT(aggreg.value.type == OTIC_TYPE_DOUBLE);
    TEST_ASSERT(aggreg.value.dval == DBL_MAX);
    
    unsigned maxRand = 128;
    oval_t randVector[maxRand];
    srand(time(0));
        
    for (typeof(maxRand) counter = 0; counter < maxRand; ++counter)
    {
        otic_oval_setlf(&randVector[counter], rand());
        aggreg.insert(&aggreg, &randVector[counter]); 
    }
    TEST_ASSERT(aggreg.get(&aggreg).dval == aggreg_min_lambda(randVector, maxRand).dval);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);

    otic_aggreg_reset(&aggreg);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.value.dval == DBL_MAX);

    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_STRING});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL);
    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_NULL});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL); 

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
    otic_aggreg_init(&aggreg, OTIC_AGGREG_MAX);

    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_MAX);
    TEST_ASSERT(aggreg.value.type == OTIC_TYPE_DOUBLE);
    TEST_ASSERT(aggreg.value.dval == DBL_MIN);
   
    unsigned maxRand = 128;
    oval_t randVector[maxRand];
    srand(time(0));
        
    for (typeof(maxRand) counter = 0; counter < maxRand; ++counter)
    {
        otic_oval_setlf(&randVector[counter], rand());
        aggreg.insert(&aggreg, &randVector[counter]); 
    }
    TEST_ASSERT(aggreg.get(&aggreg).dval == aggreg_max_lambda(randVector, maxRand).dval);
    
    otic_aggreg_reset(&aggreg);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.value.dval == DBL_MIN);

    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_STRING});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL);
    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_NULL});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL); 

    otic_aggreg_close(&aggreg);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_NULL);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.counter == 0);
    TEST_ASSERT(aggreg.insert == 0);
    TEST_ASSERT(aggreg.get == 0);
}

OTIC_TEST_CASE(aggregator, aggregator_avg)
{
    otic_aggreg_t aggreg;
    otic_aggreg_init(&aggreg, OTIC_AGGREG_AVG);

    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_AVG);
    TEST_ASSERT(aggreg.value.type == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.value.dval == 0.0);
   
    unsigned maxRand = 128;
    oval_t randVector[maxRand];
    srand(time(0));
        
    for (typeof(maxRand) counter = 0; counter < maxRand; ++counter)
    {
        otic_oval_setlf(&randVector[counter], rand());
        aggreg.insert(&aggreg, &randVector[counter]); 
    }
    TEST_ASSERT(aggreg.get(&aggreg).dval == aggreg_avg_lambda(randVector, maxRand).dval);
    otic_aggreg_reset(&aggreg);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.value.dval == 0);
    TEST_ASSERT(aggreg.counter == 0);

    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_STRING});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL);
    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_NULL});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL); 

    otic_aggreg_close(&aggreg);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_NULL);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.counter == 0);
    TEST_ASSERT(aggreg.insert == 0);
    TEST_ASSERT(aggreg.get == 0);
}

OTIC_TEST_CASE(aggregator, aggregator_first)
{
    otic_aggreg_t aggreg;
    otic_aggreg_init(&aggreg, OTIC_AGGREG_FIRST);

    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_FIRST);
    TEST_ASSERT(aggreg.value.type == OTIC_TYPE_NULL);
   
    unsigned maxRand = 8;
    oval_t randVector[maxRand];

    otic_oval_sets(&randVector[0], "This is a test", strlen("This is a test"));
    otic_oval_sets(&randVector[1], "And another one", strlen("And another one"));
    otic_oval_setlf(&randVector[2], 343.123);
    otic_oval_setlf(&randVector[3], 13.45);
    otic_oval_setd(&randVector[4], 29, 1);
    otic_oval_setd(&randVector[5], 12, 0);
    otic_oval_setn(&randVector[6]);
    otic_oval_setn(&randVector[7]);
    
    oval_t* randVector2[maxRand]; 

    srand(time(0));    
    oval_t temp;
    for (typeof(maxRand) counter = 0; counter <  maxRand; ++counter) {
        randVector2[counter] = &randVector[rand() % maxRand];
        aggreg.insert(&aggreg, randVector2[counter]);
    }
    oval_t res = aggreg.get(&aggreg);
    TEST_ASSERT(otic_oval_cmp(&res, randVector2[0]) != 0);

    otic_aggreg_reset(&aggreg);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);

    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_STRING});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_STRING);
    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_NULL});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_STRING); 

    otic_aggreg_close(&aggreg);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_NULL);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.insert == 0);
    TEST_ASSERT(aggreg.get == 0);
}

OTIC_TEST_CASE(aggregator, aggregator_last)
{
    otic_aggreg_t aggreg;
    otic_aggreg_init(&aggreg, OTIC_AGGREG_LAST);

    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_LAST);
    TEST_ASSERT(aggreg.value.type == OTIC_TYPE_NULL);
   
    unsigned maxRand = 8;
    oval_t randVector[maxRand];

    otic_oval_sets(&randVector[0], "This is a test", strlen("This is a test"));
    otic_oval_sets(&randVector[1], "And another one", strlen("And another one"));
    otic_oval_setlf(&randVector[2], 343.123);
    otic_oval_setlf(&randVector[3], 13.45);
    otic_oval_setd(&randVector[4], 29, 1);
    otic_oval_setd(&randVector[5], 12, 0);
    otic_oval_setn(&randVector[6]);
    otic_oval_setn(&randVector[7]);
    
    oval_t* randVector2[maxRand]; 

    srand(time(0));    
    oval_t temp;
    for (typeof(maxRand) counter = 0; counter <  maxRand; ++counter) {
        randVector2[counter] = &randVector[rand() % maxRand];
        aggreg.insert(&aggreg, randVector2[counter]);
    }
    oval_t res = aggreg.get(&aggreg);
    TEST_ASSERT(otic_oval_cmp(&res, randVector2[maxRand - 1]) != 0);

    otic_aggreg_reset(&aggreg);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);

    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_STRING});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_STRING);
    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_NULL});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL); 

    otic_aggreg_close(&aggreg);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_NULL);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.insert == 0);
    TEST_ASSERT(aggreg.get == 0);
}

OTIC_TEST_CASE(aggregator, aggregator_sum)
{
    otic_aggreg_t aggreg;
    otic_aggreg_init(&aggreg, OTIC_AGGREG_SUM);

    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_SUM);
    TEST_ASSERT(aggreg.value.dval == 0);
   
    unsigned maxRand = 128;
    oval_t randVector[maxRand];
    srand(time(0));
        
    for (typeof(maxRand) counter = 0; counter < maxRand; ++counter)
    {
        otic_oval_setlf(&randVector[counter], rand());
        aggreg.insert(&aggreg, &randVector[counter]); 
    }
    TEST_ASSERT(aggreg.get(&aggreg).dval == aggreg_sum_lambda(randVector, maxRand).dval);
   
    otic_aggreg_reset(&aggreg);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.value.dval == 0);

    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_STRING});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL);
    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_NULL});
    TEST_ASSERT(aggreg.get(&aggreg).type == OTIC_TYPE_NULL); 

    otic_aggreg_close(&aggreg);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_NULL);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.counter == 0);
    TEST_ASSERT(aggreg.insert == 0);
    TEST_ASSERT(aggreg.get == 0);
}

OTIC_TEST_CASE(aggregator, aggregator_count)
{
    otic_aggreg_t aggreg;
    otic_aggreg_init(&aggreg, OTIC_AGGREG_COUNT);

    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_COUNT);
    TEST_ASSERT(aggreg.value.lval == 0);

    unsigned maxRand = 128;
    oval_t randVector[maxRand];
    srand(time(0));
        
    for (typeof(maxRand) counter = 0; counter < maxRand; ++counter)
    {
        otic_oval_setlf(&randVector[counter], rand());
        aggreg.insert(&aggreg, &randVector[counter]); 
    }
    TEST_ASSERT(aggreg.get(&aggreg).lval  == maxRand);
 
    otic_aggreg_reset(&aggreg);
    TEST_ASSERT(aggreg.error == OTIC_AGGREG_ERROR_NONE);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.value.lval == 0);

    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_STRING});
    TEST_ASSERT(aggreg.get(&aggreg).lval == 1);
    
    aggreg.insert(&aggreg, &(oval_t){.type = OTIC_TYPE_NULL});
    TEST_ASSERT(aggreg.get(&aggreg).lval == 2); 

    otic_aggreg_close(&aggreg);
    TEST_ASSERT(aggreg.type == OTIC_AGGREG_NULL);
    TEST_ASSERT(otic_oval_getType(&aggreg.value) == OTIC_TYPE_NULL);
    TEST_ASSERT(aggreg.counter == 0);
    TEST_ASSERT(aggreg.insert == 0);
    TEST_ASSERT(aggreg.get == 0);
}
