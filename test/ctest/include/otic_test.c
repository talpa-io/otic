#include "otic_test.h"
#include <stdio.h>


static otic_test_descr_t* descr_first = 0;
static otic_test_descr_t* descr_last  = 0;

void otic_register_test(otic_test_descr_t* descr)
{
    if (!descr_first) {
        descr_first = descr;
        descr_last  = descr;
    } else {
        otic_test_descr_t* temp = descr_first;
        descr_first = descr;
        descr_first->next = temp;
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();
    do 
    {
       RUN_TEST(descr_first->funcPtr);
       descr_first = descr_first->next;
    } while (descr_first != 0);
    return UNITY_END();
}
