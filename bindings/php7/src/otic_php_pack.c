//
// Created by talpaadmin on 06.12.19.
//
#include "otic_php_pack.h"
#include "otic_exception.h"
#include <Zend/zend_exceptions.h>
#include <zend_string.h>
#include <otic.h>

/**
 * Useful links:
 *
 * Zend stream handling:
 * https://www.php.net/manual/de/internals2.ze1.streams.php
 */

zend_object_handlers oticPackChannel_object_handlers;

static inline oticPackChannel_object* php_oticPackChannel_obj_from_obj(zend_object* obj)
{
    return (oticPackChannel_object*)((char*)(obj) - XtOffsetOf(oticPackChannel_object, std));
}

static inline uint8_t otic_zend_stream_isOpened(php_stream* file)
{
    return file->orig_path != 0;
}

#define Z_OTICPACKCHAN_P(zv) php_oticPackChannel_obj_from_obj(Z_OBJ_P(zv))
zend_class_entry* oticPackChannel_ce;

#define TO_STR_AND_LENGTH(string) string, strlen(string)

PHP_METHOD(OticPackChannel, __construct)
{
}

PHP_METHOD(OticPackChannel, __toString)
{
    zval* id = getThis();
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern || !intern->oticPackChannel)
        return;
    char buffer[512] = {};
    sprintf(buffer, "<Class %s. Id: %hhu, Type: %hhu. State: %hhu. Error: %hhu. Total Entries: %u. Ts: %lf - %lf>",
            ZEND_NS_NAME("Otic", "OticPackChannel"), intern->oticPackChannel->info.channelId,
            intern->oticPackChannel->info.channelType, intern->oticPackChannel->base.state,
            intern->oticPackChannel->base.error, intern->oticPackChannel->totalEntries,
            intern->oticPackChannel->timeInterval.time_start == TS_NULL? 0 : intern->oticPackChannel->timeInterval.time_start,
            intern->oticPackChannel->base.timestampCurrent == TS_NULL ? 0 : (double)intern->oticPackChannel->base.timestampCurrent / OTIC_TS_MULTIPLICATOR);
    RETURN_STRING(buffer)
}

PHP_METHOD(OticPackChannel, inject)
{
    zval* id = getThis();
    double timestamp;
    otic_str_t sensorName;
    otic_str_t sensorUnit;
    zval* value;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dssz", &timestamp, &sensorName.ptr,
            &sensorName.size, &sensorUnit.ptr, &sensorUnit.size, &value) == FAILURE)
        return;
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern || !intern->oticPackChannel)
        return;
    switch (Z_TYPE_P(value))
    {
        case IS_LONG:
            if (Z_LVAL_P(value) >= 0) {
                otic_pack_channel_inject_i(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr,
                                           Z_LVAL_P(value));
            } else
                otic_pack_channel_inject_i_neg(intern->oticPackChannel, timestamp, sensorName.ptr, sensorName.ptr, -Z_LVAL_P(value));
            break;
        case IS_DOUBLE:
            otic_pack_channel_inject_d(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_DVAL_P(value));
            break;
        case IS_STRING:
            otic_pack_channel_inject_s(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_STRVAL_P(value));
            break;
        case IS_NULL:
            otic_pack_channel_inject_n(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr);
            break;
        case IS_TRUE:
            otic_pack_channel_inject_bool(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, 1);
            break;
        case IS_FALSE:
            otic_pack_channel_inject_bool(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, 0);
            break;
        default:
            otic_php_throw_oticException("Unsupported Type", 0);
    }
    if (intern->oticPackChannel->base.state == OTIC_STATE_ON_ERROR)
        otic_php_throw_libOticException(intern->oticPackChannel->base.error);
}

PHP_METHOD(OticPackChannel, flush)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern || !intern->oticPackChannel)
        return;
    if (!otic_pack_channel_flush(intern->oticPackChannel))
        otic_php_throw_libOticException(intern->oticPackChannel->base.error);
}

PHP_METHOD(OticPackChannel, getTimeInterval)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern || !intern->oticPackChannel)
        return;
    array_init(return_value);
    intern->oticPackChannel->timeInterval.time_start == TS_NULL ? add_index_null(return_value, 0)
        : add_index_double(return_value, 0, intern->oticPackChannel->timeInterval.time_start);
    intern->oticPackChannel->timeInterval.time_start == TS_NULL ? add_index_null(return_value, 1)
        : add_index_double(return_value, 1, (double)intern->oticPackChannel->base.timestampCurrent / OTIC_TS_MULTIPLICATOR);
}

PHP_METHOD(OticPackChannel, getSensorsList)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern || !intern->oticPackChannel)
        return;
    if (otic_base_getState(&intern->oticPackChannel->base) == OTIC_STATE_CLOSED)
        otic_php_throw_oticException("Channel already closed", -2);
    array_init(return_value);
    size_t counter;
    for (counter = 0; counter < OTIC_PACK_CACHE_SIZE; ++counter)
    {
        otic_entry_t* current = intern->oticPackChannel->cache[counter];
        while (current) {
            add_next_index_string(return_value, current->name);
            current = current->next;
        }
    }
}

PHP_METHOD(OticPackChannel, getStats)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern || !intern->oticPackChannel)
        return;
    array_init(return_value);
#if OTIC_STATS
#define ADD2ARRAY(key, val) add_assoc_long(return_value, #key, val);
    ADD2ARRAY(blocksWritten, intern->oticPackChannel->stats.blocksWritten)
    ADD2ARRAY(typeUnmodified, intern->oticPackChannel->stats.blocksWritten)
    ADD2ARRAY(typeInteger, intern->oticPackChannel->stats.type_integer)
    ADD2ARRAY(typeString, intern->oticPackChannel->stats.type_string)
    ADD2ARRAY(typeBool, intern->oticPackChannel->stats.type_bool)
    ADD2ARRAY(typeDouble, intern->oticPackChannel->stats.type_double)
    ADD2ARRAY(typeArray, intern->oticPackChannel->stats.type_array)
    ADD2ARRAY(typeObject, intern->oticPackChannel->stats.type_object)
    ADD2ARRAY(typeNull, intern->oticPackChannel->stats.type_null)
    ADD2ARRAY(typeTimestampSets, intern->oticPackChannel->stats.time_sets)
    ADD2ARRAY(typeTimestampShifts, intern->oticPackChannel->stats.time_shifts)
    ADD2ARRAY(typeColsAssigned, intern->oticPackChannel->stats.cols_assigned)
    ADD2ARRAY(rowsRead, intern->oticPackChannel->base.rowCounter)
#undef ADD2ARRAY
#endif
}

PHP_METHOD(OticPackChannel, resizeBucket)
{
    long size;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &size) == FAILURE)
        return;
    if (size < 0 || size > UINT32_MAX)
        otic_php_throw_oticException("Valid Bucket Size Range: [0, UINT32_MAX]", -6);
    zval* id = getThis();
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern || !intern->oticPackChannel)
        return;
    if (!otic_pack_channel_resizeBucket(intern->oticPackChannel, size))
        otic_php_throw_libOticException(intern->oticPackChannel->base.error);
}

PHP_METHOD(OticPackChannel, close)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern || !intern->oticPackChannel)
        return;
    if (!otic_pack_channel_close(intern->oticPackChannel))
        otic_php_throw_libOticException(intern->oticPackChannel->base.error);
}

ZEND_BEGIN_ARG_INFO_EX(argInfo_oticPackInj, 0, 0, 1)
                ZEND_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
                ZEND_ARG_TYPE_INFO(0, sensorName, IS_STRING, 0)
                ZEND_ARG_TYPE_INFO(0, sensorUnit, IS_STRING, 1)
                ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_resizeInt, 0, 0, 1)
                ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
ZEND_END_ARG_INFO()

const zend_function_entry oticPackChannel_methods[] = {
        PHP_ME(OticPackChannel, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticPackChannel, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, inject, argInfo_oticPackInj, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, getTimeInterval, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, getSensorsList, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, getStats, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, resizeBucket, argInfo_resizeInt, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, flush, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, close, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_object* oticPackChannel_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticPackChannel_object* intern = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &oticPackChannel_object_handlers;
    return &intern->std;
}

void oticPackChannel_object_destroy(zend_object* object)
{
    zend_objects_destroy_object(object);
}

void oticPackChannel_object_free(zend_object* object)
{
    zend_object_std_dtor(object);
}


zend_object_handlers oticPack_object_handlers;

static inline oticPack_object* php_oticPack_obj_from_obj(zend_object* obj)
{
    return (oticPack_object*)((char*)(obj) - XtOffsetOf(oticPack_object, std));
}

#define Z_OTICPACK_P(zv) php_oticPack_obj_from_obj(Z_OBJ_P((zv)))

zend_class_entry* oticPack_ce;

static uint8_t oticPack_flusher(uint8_t* content, size_t size, void* data)
{
    php_stream_write((php_stream*)data, (const char*)content, size);
    return 1;
}

PHP_METHOD(OticPack, __construct)
{
    zval* id = getThis();
    zval* fileOut;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &fileOut) == FAILURE)
        RETURN_FALSE
    oticPack_object* intern = Z_OTICPACK_P(id);
    if (intern) {
        php_stream* stream;
        php_stream_from_zval(stream, fileOut);
        intern->oticPack = emalloc(sizeof(otic_pack_t));
        if (!otic_pack_init(intern->oticPack, 0x00, oticPack_flusher, stream))
            otic_php_throw_libOticException(intern->oticPack->error);
    }
}

PHP_METHOD(OticPack, __toString)
{
    char buffer[512] = {};
    zval* id = getThis();
    oticPack_object* intern = Z_OTICPACK_P(id);
    if (!intern)
        return;
    sprintf(buffer, "<Class: %s. Total Channels: %hhu. State: %u>", ZEND_NS_NAME("Otic", "OticPack"), otic_pack_getTotalAmountOfChannel(intern->oticPack), intern->oticPack->state);
    RETURN_STRING(buffer)
}

PHP_METHOD(OticPack, __destruct)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPack_object* intern = Z_OTICPACK_P(id);
    if (intern) {
        if (intern->oticPack->state == OTIC_STATE_CLOSED)
            return;
        if (!otic_zend_stream_isOpened(intern->oticPack->data))
            otic_php_throw_oticException("Cannot close Pack because output file was already closed", -1);
        otic_pack_close(intern->oticPack);
    }
}

PHP_METHOD(OticPack, defineChannel)
{
    long channelId, channelType, channelFeatures;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &channelId, &channelType, &channelFeatures) == FAILURE)
        return;
    if (channelId < 0 || channelId > UINT8_MAX)
        otic_php_throw_oticException("Invalid ChannelID! Reason: Valid Range (int): 0 - 255", -6);
    if (channelType != OTIC_CHANNEL_TYPE_SENSOR && channelType != OTIC_CHANNEL_TYPE_BINARY)
	otic_php_throw_oticException("Invalid Channel Type", -6);
    zval* id = getThis();
    oticPack_object *intern = Z_OTICPACK_P(id);
    if (!intern)
        return;
    oticPackChannel_object* channel = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(oticPackChannel_ce));
    channel->oticPackChannel = otic_pack_defineChannel(intern->oticPack, channelType, channelId, 0x00, 0);
    if (!channel->oticPackChannel) {
        efree(channel);
        otic_php_throw_libOticException(intern->oticPack->error);
	    return;
    }
    zend_object_std_init(&channel->std, oticPackChannel_ce TSRMLS_CC);
    object_properties_init(&channel->std, oticPackChannel_ce);
    channel->std.handlers = &oticPackChannel_object_handlers;
    RETURN_OBJ(&channel->std)
}

PHP_METHOD(OticPack, clearErrorFlag)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticPack_object* intern = Z_OTICPACK_P(id);
    if (!intern)
	return;
    otic_pack_clearErrorFlag(intern->oticPack);    
}

PHP_METHOD(OticPack, closeChannel)
{
    zval* id = getThis();
    long channelID;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelID) == FAILURE)
        return;
    if (channelID < 0 || channelID > UINT8_MAX)
        zend_throw_exception(oticExceptions_ce, "Invalid ChannelID", 0);
    oticPack_object *intern = Z_OTICPACK_P(id);
    if (intern) {
        otic_pack_closeChannel(intern->oticPack, (uint8_t)channelID);
        if (intern->oticPack->state == OTIC_STATE_ON_ERROR)
            otic_php_throw_libOticException(intern->oticPack->error);
    }
}

PHP_METHOD(OticPack, flush)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPack_object* intern = Z_OTICPACK_P(id);
    if (intern) {
        otic_pack_flush(intern->oticPack);
        if (intern->oticPack->state == OTIC_STATE_ON_ERROR)
            otic_php_throw_libOticException(intern->oticPack->error);
    }
}

PHP_METHOD(OticPack, close)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPack_object* intern = Z_OTICPACK_P(id);
    if (intern) {
        if (intern->oticPack->state == OTIC_STATE_CLOSED)
            return;
        if (!otic_zend_stream_isOpened(intern->oticPack->data))
            otic_php_throw_oticException("Close Failure! Reason: Output Stream already closed.", -1);
        otic_pack_close(intern->oticPack);
    }
}

ZEND_BEGIN_ARG_INFO_EX(argInfo_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_oticPackConst, 0, 0, 1)
    ZEND_ARG_INFO(0, outputFile)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_oticPackDefChan, 0, 0, 1)
                ZEND_ARG_INFO(0, channelId)
                ZEND_ARG_INFO(0, channelType)
                ZEND_ARG_INFO(0, channelFeatures)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_oticPackCloseChan, 0, 0, 1)
                ZEND_ARG_INFO(0, channelId)
ZEND_END_ARG_INFO()

const zend_function_entry oticPack_methods[] = {
        PHP_ME(OticPack, __construct, argInfo_oticPackConst, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticPack, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(OticPack, defineChannel, argInfo_oticPackDefChan, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, closeChannel, argInfo_oticPackCloseChan, ZEND_ACC_PUBLIC)
	PHP_ME(OticPack, clearErrorFlag, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, flush, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, close, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_object* oticPack_object_new(zend_class_entry* ce TSRMLS_DC)
{
   oticPack_object* intern = (oticPack_object*)ecalloc(1, sizeof(oticPack_object) + zend_object_properties_size(ce));
   zend_object_std_init(&intern->std, ce TSRMLS_CC);
   object_properties_init(&intern->std, ce);
   intern->std.handlers = &oticPack_object_handlers;
   return &intern->std;
}

void oticPack_object_destroy(zend_object* object)
{
    oticPack_object* myObj = (oticPack_object*)((char*)object - XtOffsetOf(oticPack_object, std));
    zend_objects_destroy_object(object);
}

void oticPack_object_free(zend_object* object)
{
    oticPack_object* myObj = (oticPack_object*)((char*)object - XtOffsetOf(oticPack_object, std));
    efree(myObj->oticPack);
    zend_object_std_dtor(object);
}

#undef TO_STR_AND_LENGTH
#undef Z_OTICPACK_P
#undef Z_OTICPACKCHAN_P
