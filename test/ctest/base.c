#include <limits.h>
#include <assert.h>
#include "core/base.h"
#include "../test_asserts.h"


static void test_leb128_unsigned(void)
{
    uint32_t counter = 0;
    uint8_t buffer[10] = {};
    uint32_t temp = 0;
    while (counter < 100000000)
    {
        leb128_encode_unsigned(counter, buffer);
        leb128_decode_unsigned(buffer, &temp);
        assert(counter == temp);
        counter++;
    }
    leb128_encode_unsigned(UINT32_MAX, buffer);
    leb128_decode_unsigned(buffer, &temp);
    assert(UINT32_MAX == temp);
}

static void test_leb128_signed(void)
{
    int32_t counter = -50000000;
    uint8_t buffer[10] = {};
    int64_t temp = 0;
    while (counter < 50000000)
    {
        leb128_encode_signed(counter, buffer);
        leb128_decode_signed(buffer, &temp);
        assert(counter == temp);
        counter++;
    }
//  Jeez! C needs lambda functions
//  GNU C supports embedded Functions, but this should be made standard IMO
    leb128_encode_signed(INT32_MAX, buffer);
    leb128_decode_signed(buffer, &temp);
    assert(INT32_MAX == temp);
    leb128_encode_signed(INT32_MIN, buffer);
    leb128_decode_signed(buffer, &temp);
    assert(INT32_MIN == temp);
    leb128_encode_signed(INT64_MAX, buffer);
    leb128_decode_signed(buffer, &temp);
    assert(temp == INT64_MAX);
    leb128_encode_signed(INT64_MIN, buffer);
    leb128_decode_signed(buffer, &temp);
    assert(temp == INT64_MIN);
}

static void test_base_init(void)
{
    otic_base_t oticBase;
    assert(!otic_base_init(0));
    assert(otic_base_init(&oticBase));
    assert(oticBase.top == oticBase.cache);
    assert(oticBase.timestamp_start == oticBase.timestamp_current == 0);
    assert(oticBase.error == 0);
    assert(oticBase.state == OTIC_STATE_OPENED);
    assert(oticBase.rowCounter == 0);
}

static void test_error_handling(void)
{
    otic_base_t oticBase;
    otic_base_init(&oticBase);
    otic_base_setError(&oticBase, OTIC_ERROR_INVALID_FILE);
    assert(oticBase.error == OTIC_ERROR_INVALID_FILE);
    assert(OTIC_ERROR_INVALID_FILE == otic_base_getError(&oticBase));
}

static void test_state_handling(void)
{
    otic_base_t oticBase;
    otic_base_init(&oticBase);
    otic_base_setState(&oticBase, OTIC_STATE_ON_ERROR);
    assert(oticBase.state == OTIC_STATE_ON_ERROR);
    assert(OTIC_STATE_ON_ERROR == otic_base_getState(&oticBase));
}

int main()
{
    test_base_init();
    test_error_handling();
    test_state_handling() ;
    test_leb128_unsigned();
    test_leb128_signed();
    return 0;
}