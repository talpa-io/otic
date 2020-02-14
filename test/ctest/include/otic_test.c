#include "otic_test.h"


static otic_test_descr_t* descr_first = 0;

void otic_register_test(otic_test_descr_t* descr)
{
    if (!descr_first) {
        descr_first = descr;
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

static void runTest(otic_test_descr_t* descr)
{
    Unity.CurrentTestName = descr->testName;
    Unity.TestFile = descr->fileName;
    Unity.NumberOfTests++;
    UNITY_CLR_DETAILS();
    UNITY_EXEC_TIME_START();
    if (TEST_PROTECT())
    {
        setUp();
        descr->funcPtr();
    }
    if (TEST_PROTECT())
    {
        tearDown();
    }
    UNITY_EXEC_TIME_STOP();
    UnityConcludeTest();
}

int main(void)
{
    UNITY_BEGIN();
    while (descr_first != 0)
    {
        runTest(descr_first);
        descr_first = descr_first->next;
    }
    return UNITY_END();
}