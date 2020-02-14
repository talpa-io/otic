#include <limits.h>
#include <string.h>
#include "core/base.h"
#include "otic_test.h"


static uint8_t numbLeb128BytesFinder_u(uint64_t value) {
    uint8_t counter = 0;
    do {
        ++counter;
    } while (value >>= 7u);
    return counter;
}

static uint8_t numbLeb128BytesFinder_s(int64_t value) {
    uint8_t counter = 0;
    do {
        ++counter;
    } while (value >>= 7u);
    return counter;
}

static void leb128_unsigned_lambda(uint64_t value, uint8_t* buffer)
{
    uint8_t totalLEB128Bytes = numbLeb128BytesFinder_u(value);
    uint64_t decoded;
    TEST_ASSERT_EQUAL_HEX8(totalLEB128Bytes, leb128_encode_unsigned(value, buffer));
    TEST_ASSERT_EQUAL_HEX8(totalLEB128Bytes, leb128_decode_unsigned(buffer, &decoded));
    TEST_ASSERT_EQUAL_HEX32(value, decoded);
}

// TODO: Check matching total encoded decoded bytes
static uint8_t leb128_signed_lambda(int64_t value, uint8_t* buffer)
{
    int64_t decoded;
    leb128_encode_signed(value, buffer);
    leb128_decode_signed(buffer, &decoded);
    TEST_ASSERT_EQUAL_INT64(value, decoded);
}

OTIC_TEST_CASE(otic_base, leb128_unsigned)
{
    uint8_t buffer[12] = {};
    for (uint64_t counter = 0; counter < 10000000; ++counter)
        leb128_unsigned_lambda(counter, buffer);
    leb128_unsigned_lambda(UINT64_MAX, buffer);
    leb128_unsigned_lambda(15393578146222, buffer);
}

OTIC_TEST_CASE(otic_base, leb128_signed)
{
    uint8_t buffer[10] = {};
    for (int64_t counter = -5000000; counter < 5000000; ++counter)
        leb128_signed_lambda(counter, buffer);
    leb128_signed_lambda(INT64_MAX, buffer);
    leb128_signed_lambda(INT64_MIN, buffer);
}

OTIC_TEST_CASE(otic_base, assert_sizes)
{
    TEST_ASSERT_EQUAL_size_t((OTIC_MAGIC_SIZE + 2) * sizeof(char), sizeof(otic_header_t));
    TEST_ASSERT_EQUAL_size_t(2 * sizeof(uint8_t), sizeof(otic_meta_data_t));
    TEST_ASSERT_EQUAL_size_t(sizeof(uint8_t) + sizeof(uint32_t), sizeof(otic_payload_t));
}

// TODO: Timestamp handling
OTIC_TEST_CASE(otic_base, base)
{
    otic_base_t oticBase;
    otic_base_init(&oticBase, 16384);
    TEST_ASSERT(oticBase.error == OTIC_ERROR_NONE)
    TEST_ASSERT(oticBase.state == OTIC_STATE_OPENED)
    TEST_ASSERT(oticBase.timestampStart == TS_NULL)
    TEST_ASSERT(oticBase.top == oticBase.cache)
    TEST_ASSERT(oticBase.timestampCurrent == TS_NULL)

    TEST_ASSERT(otic_base_getError(&oticBase) == OTIC_ERROR_NONE)
    otic_base_setError(&oticBase, OTIC_ERROR_ROW_COUNT_MISMATCH);
    TEST_ASSERT(otic_base_getError(&oticBase) == OTIC_ERROR_ROW_COUNT_MISMATCH)

    TEST_ASSERT(otic_base_getState(&oticBase) == OTIC_STATE_OPENED)
    otic_base_setState(&oticBase, OTIC_STATE_ON_ERROR);
    TEST_ASSERT(otic_base_getState(&oticBase) == OTIC_STATE_ON_ERROR)

    otic_base_close(&oticBase);
    TEST_ASSERT(oticBase.state == OTIC_STATE_CLOSED)
}

OTIC_TEST_CASE(otic_base, ostr)
{
    otic_str_t* str = otic_setStr("Hallo World");
    TEST_ASSERT(str)
    TEST_ASSERT(str->ptr)
    TEST_ASSERT_EQUAL_STRING("Hallo World", str->ptr);
    TEST_ASSERT_EQUAL_size_t(strlen("Hallo World"), str->size);

    otic_updateStr(str, "This is a test");
    TEST_ASSERT_EQUAL_STRING("This is a test", str->ptr);
    TEST_ASSERT_EQUAL_size_t(strlen("This is a test"), str->size);

    otic_freeStr(str);
    TEST_ASSERT(!str->ptr)
}

OTIC_TEST_CASE(otic_base, oval)
{
    oval_t oval;
    otic_oval_setd(&oval, 12345, 1);
    TEST_ASSERT_EQUAL_HEX32(oval.val.lval, 12345);
    TEST_ASSERT(oval.type == OTIC_TYPE_INT_NEG)
    otic_oval_setd(&oval, 12345, 0);
    TEST_ASSERT_EQUAL_HEX32(oval.val.lval, 12345);
    TEST_ASSERT(oval.type == OTIC_TYPE_INT_POS)
    otic_oval_setdp(&oval, 98765);
    TEST_ASSERT_EQUAL_HEX32(oval.val.lval, 98765);
    TEST_ASSERT(oval.type == OTIC_TYPE_INT_POS)
    otic_oval_setdn(&oval, 2345);
    TEST_ASSERT_EQUAL_HEX32(oval.val.lval, 2345);
    TEST_ASSERT(oval.type == OTIC_TYPE_INT_NEG)
    otic_oval_setlf(&oval, 12345.6);
    TEST_ASSERT_EQUAL_DOUBLE(oval.val.dval, 12345.6);
    TEST_ASSERT(oval.type == OTIC_TYPE_DOUBLE)
    otic_oval_sets(&oval, "Hallo World", strlen("Hallo World"));
    TEST_ASSERT_EQUAL_STRING(oval.val.sval.ptr, "Hallo World");
    TEST_ASSERT(oval.type == OTIC_TYPE_STRING)
    otic_oval_setn(&oval);
    TEST_ASSERT(oval.type == OTIC_TYPE_NULL)
}
