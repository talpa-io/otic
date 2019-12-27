#include <unity.h>

typedef struct otic_test_descr_t otic_test_descr_t;

struct otic_test_descr_t
{
    const char* testCategory;
    const char* testName;
    void (*funcPtr)(void);
    const char* fileName;
    size_t lineNumber;
    uint8_t totalTestCounter;
    const char** testFuncNames;
    otic_test_descr_t* next;
};

void otic_register_test(otic_test_descr_t* descr);  

typedef void(*empty_func_ptr)(void);

#define COMBINE(val1, val2) val1##val2

#define OTIC_TEST_CASE(_testCategory, _testName) \
    static void otic_test_##_testCategory##_testName(void); \
    static __attribute__((constructor)) void otic_testHelper_##_testCategory##_testName(void)\
    {\
        static empty_func_ptr empty_funcs[] = {&otic_test_##_testCategory##_testName};\
        static otic_test_descr_t otic_test_descr_##_testCategory##_testName = {\
            .testCategory = "##_testCategory",\
            .testName     = "##_testName",\
            .funcPtr      = otic_test_##_testCategory##_testName,\
            .fileName     = __FILE__,   \
            .lineNumber   = __LINE__,   \
            .totalTestCounter = 1   ,   \
            .testFuncNames = 0,         \
            .next          = 0,         \
        };      \
        otic_register_test(&otic_test_descr_##_testCategory##_testName);\
    }\
    static void otic_test_##_testCategory##_testName(void)
