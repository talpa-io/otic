//
// Created by talpaadmin on 06.12.19.
//

#include "otic_php_pack.h"
#include "oticException.h"
#include <Zend/zend_exceptions.h>


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

#define Z_OTICPACKCHAN_P(zv) php_oticPackChannel_obj_from_obj(Z_OBJ_P(zv))
zend_class_entry* oticPackChannel_ce;

PHP_METHOD(OticPackChannel, __toString)
{
    RETURN_STRING(ZEND_NS_NAME("otic", "OticPackChannel"))
}

PHP_METHOD(OticPackChannel, inject)
{
    zval* id = getThis();
    double timestamp;
    otic_str_t sensorName;
    otic_str_t sensorUnit;
    zval* value;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dssz", &timestamp, &sensorName.ptr, &sensorName.size, &sensorUnit.ptr, &sensorUnit.size, &value) == FAILURE)
        return;
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern)
        return;
    switch (Z_TYPE_P(value))
    {
        case IS_LONG:
            if (Z_LVAL_P(value) >= 0)
                otic_pack_channel_inject_i(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_LVAL_P(value));
            else
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
        default:
            zend_throw_exception(oticExceptions_ce, "Unsupported Type", 0);
    }
    if (intern->oticPackChannel->base.state == OTIC_STATE_ON_ERROR)
        zend_throw_exception(libOticExceptions_ce, "", intern->oticPackChannel->base.error);
}

PHP_METHOD(OticPackChannel, flush)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern)
        return;
    if (!otic_pack_channel_flush(intern->oticPackChannel))
        zend_throw_exception(libOticExceptions_ce, "", intern->oticPackChannel->base.error);
}

PHP_METHOD(OticPackChannel, close)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPackChannel_object* intern = Z_OTICPACKCHAN_P(id);
    if (!intern)
        return;
    if (!otic_pack_channel_close(intern->oticPackChannel))
        zend_throw_exception(libOticExceptions_ce, "", intern->oticPackChannel->base.error);
}

ZEND_BEGIN_ARG_INFO_EX(argInfo_oticPackInj, 0, 0, 1)
                ZEND_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
                ZEND_ARG_INFO(0, sensorName)
                ZEND_ARG_INFO(0, sensorUnit)
                ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

const zend_function_entry oticPackChannel_methods[] = {
        PHP_ME(OticPackChannel, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, inject, argInfo_oticPackInj, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, flush, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackChannel, close, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_object* oticPackChannel_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticPackChannel_object* intern = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(ce));
    object_properties_init(&intern->std, ce);
    object_properties_init(&intern->std, ce TSRMLS_CC);
    intern->std.handlers = &oticPackChannel_object_handlers;
    return &intern->std;
}

void oticPackChannel_object_destroy(zend_object* object)
{
    oticPackChannel_object* myObj = (oticPackChannel_object*)((char*)object - XtOffsetOf(oticPackChannel_object, std));
    zend_objects_destroy_object(object);
}

void oticPackChannel_object_free(zend_object* object)
{
    oticPackChannel_object* myObj = (oticPackChannel_object*)((char*)object - XtOffsetOf(oticPackChannel_object, std));
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
        if (!otic_pack_init(intern->oticPack, oticPack_flusher, stream))
            zend_throw_exception(libOticExceptions_ce, "", intern->oticPack->error);
    }
}

PHP_METHOD(OticPack, __toString)
{
    RETURN_STRING(ZEND_NS_NAME("Otic", "OticPack"))
}

PHP_METHOD(OticPack, __destruct)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPack_object* intern = Z_OTICPACK_P(id);
    if (intern) {
        // TODO: Check if the file was already close (throw an exception, if this happens to be the case)
        if (intern->oticPack->state != OTIC_STATE_CLOSED)
            otic_pack_close(intern->oticPack);
    }
}

PHP_METHOD(OticPack, defineChannel)
{
    zval* id = getThis();
    long channelId, channelType, channelFeatures;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &channelId, &channelType, &channelFeatures) == FAILURE)
        return;
    if (channelId < 0 || channelId > UINT8_MAX)
        zend_throw_exception(oticExceptions_ce, "Invalid ChannelID", 0);
    oticPack_object *intern = Z_OTICPACK_P(id);
    if (!intern)
        return;
    oticPackChannel_object* channel = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(oticPackChannel_ce));
    zend_object_std_init(&channel->std, oticPackChannel_ce TSRMLS_CC);
    channel->std.handlers = &oticPackChannel_object_handlers;
    channel->oticPackChannel = otic_pack_defineChannel(intern->oticPack, channelType, channelId, 0x00);
    RETURN_OBJ(&channel->std)
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
        otic_pack_closeChannel(intern->oticPack, (uint8_t)id);
        if (intern->oticPack->state == OTIC_STATE_ON_ERROR)
            zend_throw_exception(libOticExceptions_ce, "", intern->oticPack->error);
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
            zend_throw_exception(libOticExceptions_ce, "", intern->oticPack->error);
    }
}

PHP_METHOD(OticPack, close)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPack_object* intern = Z_OTICPACK_P(id);
    if (intern)
        // TODO: Make sure that the file is still opened
        otic_pack_close(intern->oticPack);
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