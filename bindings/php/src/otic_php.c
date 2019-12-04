#ifdef HAVE_TIMELIB_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <otic.h>
#include <Zend/zend.h>
#include <Zend/zend_types.h>
#include <Zend/zend_exceptions.h>

// Useful Links concerning Zend (I mean, there are so few pages about zend-engine that
// you are bound to stumble on them on your first google search.):
// https://wiki.php.net/phpng-upgrading
// https://www.php.net/manual/de/internals2.ze1.zendapi.php
// http://www.phpinternalsbook.com/php5/classes_objects/simple_classes.html
// http://www.phpinternalsbook.com/php7/internal_types/zvals/basic_structure.html
// https://www.php.net/manual/de/internals2.ze1.streams.php for streams
// https://www.maplenerds.com/blog/2018/6/14/writing-php-72-extensions-with-c for an example

// TODO: Update the OticUnpackStream method parameters
// TODO: Port C error Handling to PHP

#define PHP_OTIC_EXTVER "1.0"
#define PHP_OTIC_EXTNAME "otic"

ZEND_BEGIN_ARG_INFO_EX(argInfo_defineChannel, 0, 0, 3)
                ZEND_ARG_INFO(0, channelId)
                ZEND_ARG_INFO(0, channelType)
                ZEND_ARG_INFO(0, channelOptions)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_otic_unpack_defineChannel, 0, 0, 2)
                ZEND_ARG_INFO(0, channelId)
                ZEND_ARG_INFO(0, flusher)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_otic_unpack_closeChannel, 0, 0, 1)
                ZEND_ARG_INFO(0, channelId)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_closeChannel, 0, 0, 1)
                ZEND_ARG_INFO(0, channelId)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(FUNCTION_PARAM, 0, 0, 1)
                ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(FUNCTION_INJECT, 0, 0, 4)
                ZEND_ARG_INFO(0, timestamp)
                ZEND_ARG_INFO(0, sensorName)
                ZEND_ARG_INFO(0, sensorValue)
                ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(OTIC_STREAM_SINGLE_ARG, 0, 0, 1)
                ZEND_ARG_INFO(0, fileOut)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(OTIC_STREAM_INPUT_FILE_ARG, 0, 0, 1)
                ZEND_ARG_INFO(0, fileIn)
ZEND_END_ARG_INFO()


typedef struct
{
    zval* data;
    zend_fcall_info func;
} otic_data_t;

static uint8_t php_otic_flusher(uint8_t* ptr, size_t size, void* data)
{
    otic_data_t* oticData = (otic_data_t*)data;
    zval ret;
    zval params[3];
    oticData->func.param_count = 3;
    oticData->func.retval = &ret;
    oticData->func.params = params;
    ZVAL_STRING(&params[0], ptr);
    ZVAL_LONG(&params[1], size)
    ZVAL_ZVAL(&params[2], oticData->data, 1, 1);
    if (zend_call_function(&oticData->func, 0) != SUCCESS)
        return 0;
    return Z_TYPE_P(&ret) == IS_TRUE;
}

zend_object_handlers oticPack_handlers;
zend_object_handlers oticPackStream_handlers;
zend_object_handlers oticPackChannel_handlers;
zend_object_handlers oticUnpack_handlers;
zend_object_handlers oticUnpackStream_handlers;
zend_object_handlers oticUnpackChannel_handlers;

typedef struct
{
    otic_pack_t* oticPack;
    zend_object std;
} oticPack_object;

typedef struct
{
    otic_pack_channel_t* oticPackChannel;
    zend_object std;
} oticPackChannel_object;

typedef struct
{
    otic_unpack_t* oticUnpack;
    zend_object std;
} oticUnpack_object;

typedef struct
{
} oticUnpackChannel_object;

static inline oticPack_object* php_oticPack_obj_from_obj(zend_object* obj)
{
    return (oticPack_object*)((char*)(obj) - XtOffsetOf(oticPack_object, std));
}

static inline oticPackChannel_object* php_oticPackChannel_obj_from_obj(zend_object* obj)
{
    return (oticPackChannel_object*)((char*)(obj) - XtOffsetOf(oticPackChannel_object, std));
}

static inline oticUnpack_object* php_oticUnpack_obj_from_obj(zend_object* obj)
{
    return (oticUnpack_object*)((char*)(obj) - XtOffsetOf(oticUnpack_object, std));
}

#define Z_TSTOBJ_P(zv) php_oticPack_obj_from_obj(Z_OBJ_P((zv)))
#define Z_OTICSTREAM_OBJ_P(zv) php_oticPackStream_obj_from_obj(Z_OBJ_P((zv)))
#define Z_OTICPACKCHANNELOBJ_P(zv) php_oticPackChannel_obj_from_obj(Z_OBJ_P((zv)))
#define Z_OUPOBJ_P(zv) php_oticUnpack_obj_from_obj(Z_OBJ_P((zv)))

zend_class_entry* oticPack_ce;
zend_class_entry* oticPackChannel_ce;
zend_class_entry* oticPackStream_ce;
zend_class_entry* oticUnpack_ce;
zend_class_entry* oticUnpackStream_ce;

PHP_METHOD(OticPackChannel, __construct)
{
    zval* id = getThis();
    oticPackChannel_object *intern = Z_OTICPACKCHANNELOBJ_P(id);
    if (intern)
        intern->oticPackChannel = emalloc(sizeof(otic_pack_channel_t));
}

PHP_METHOD(OticPackChannel, inject)
{
    zval* id = getThis();
    otic_str_t sensorName;
    otic_str_t sensorUnit;
    double timestamp = 0;
    zval* value = 0;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dssz", &timestamp, &sensorName.ptr, &sensorName.size, &sensorUnit.ptr, &sensorUnit.size, &value) == FAILURE)
        return;
    oticPackChannel_object* intern = Z_OTICPACKCHANNELOBJ_P(id);
    if (!intern)
        return;
    switch(Z_TYPE_P(value))
    {
        case IS_LONG:
            if (value >= 0)
                RETURN_BOOL(otic_pack_channel_inject_i(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_LVAL_P(value)))
            else
                RETURN_BOOL(otic_pack_channel_inject_i_neg(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, -Z_LVAL_P(value)))
        case IS_DOUBLE:
            RETURN_BOOL(otic_pack_channel_inject_d(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_DVAL_P(value)))
        case IS_STRING:
            RETURN_BOOL(otic_pack_channel_inject_s(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_STRVAL_P(value)))
        case IS_NULL:
            RETURN_BOOL(otic_pack_channel_inject_n(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr))
        default:
            zend_throw_error(zend_ce_exception, "Invalid Type", 0);
    }
}

PHP_METHOD(OticPackChannel, flush)
{
    zval* id = getThis();
    oticPackChannel_object* intern;
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_FALSE
    if (!(intern = Z_OTICPACKCHANNELOBJ_P(id)))
        return;
    RETURN_BOOL(otic_pack_channel_flush(intern->oticPackChannel))
}

PHP_METHOD(OticPackChannel, __destruct)
{
}

const zend_function_entry oticPackChannel_methods[] = {
    PHP_ME(OticPackChannel, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(OticPackChannel, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    PHP_ME(OticPackChannel, inject, FUNCTION_INJECT, ZEND_ACC_PUBLIC)
    PHP_ME(OticPackChannel, flush, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

zend_object* oticPackChannel_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticPackChannel_object* intern = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce TSRMLS_CC);
    intern->std.handlers = &oticPackChannel_handlers;
    return &intern->std;
}

static void oticPackChannel_object_destroy(zend_object* object)
{
    oticPackChannel_object* my_obj = (oticPackChannel_object*)((char*)object - XtOffsetOf(oticPackChannel_object, std));
    zend_objects_destroy_object(object);
}

static void oticPackChannel_object_free(zend_object* object)
{
    oticPackChannel_object* my_obj = (oticPackChannel_object*)((char*)object - XtOffsetOf(oticPackChannel_object, std));
    // TODO: Check Reliability. Segfault? Sanitizer
    //efree(my_obj->oticPackChannel);
    zend_object_std_dtor(object);
}

PHP_METHOD(OticPack, __construct)
{
    zval* id = getThis();
    zend_fcall_info_cache fcc;
    otic_data_t* data_o = malloc(sizeof(otic_data_t));
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "fr", &data_o->func, &fcc, &data_o->data) == FAILURE)
        RETURN_FALSE
    oticPack_object* intern;
    if (!(intern = Z_TSTOBJ_P(id)))
        RETURN_FALSE
    intern->oticPack = emalloc(sizeof(otic_pack_t));
    zval* newFooValue = 0;

//    zend_update_property(Z_OBJCE_P(id), id, "fooValue", strlen("fooValue"), newFooValue TSRMLS_CC);
//    zend_update_property(Z_OBJCE_P(id), id, prop_name, prop_size, newFooValue TSRMLS_CC);
    RETURN_BOOL(otic_pack_init(intern->oticPack, php_otic_flusher, data_o))
}

PHP_METHOD(OticPack, __toString)
{
    zval* id = getThis();
    oticPack_object* intern;
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_NULL()
    intern = Z_TSTOBJ_P(id);
    if (intern)
        RETURN_STRING(ZEND_NS_NAME("Otic", "OticPack"))
}

PHP_METHOD(OticPack, close) {
    zval *id = getThis();
    oticPack_object *intern;
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_NULL()
    intern = Z_TSTOBJ_P(id);
    if (intern)
        otic_pack_close(intern->oticPack);
}

PHP_METHOD(OticPack, flush)
{
    zval* id = getThis();
    oticPack_object* intern;
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_NULL()
    intern = Z_TSTOBJ_P(id);
    if (intern)
        RETURN_BOOL(otic_pack_flush(intern->oticPack))
}

PHP_METHOD(OticPack, defineChannel)
{
    zval* id = getThis();
    oticPack_object* intern;
    long channelId;
    zend_string* channelType;
    long channelFeatures;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lSl", &channelId, &channelType, &channelFeatures) == FAILURE)
        return;
    intern = Z_TSTOBJ_P(id);
    if (!intern)
        return;
    channel_type_e type;
    if (zend_string_equals_literal(channelType, "sensor"))
        type = OTIC_CHANNEL_TYPE_SENSOR;
    else if (zend_string_equals_literal(channelType, "binary"))
        type = OTIC_CHANNEL_TYPE_BINARY;
    else
        RETURN_FALSE
    zend_string_release(channelType);

    // TODO: zend_fetch_resource fof file handle
    zval value;
    ZVAL_BOOL(&value, 1);
    zval_dtor(&value);
    oticPackChannel_object* intern2 = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(oticPackChannel_ce));
    zend_object_std_init(&intern2->std, oticPackChannel_ce TSRMLS_CC);
    object_properties_init(&intern2->std, oticPackChannel_ce TSRMLS_CC);
    intern2->std.handlers = &oticPackChannel_handlers;
    intern2->oticPackChannel = otic_pack_defineChannel(intern->oticPack, type, channelId, 0x00);
    RETURN_OBJ(&intern2->std)
}

PHP_METHOD(OticPack, closeChannel)
{
    zval* id = getThis();
    oticPack_object* intern;
    long channelId;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelId) == FAILURE)
        return;
    if ((intern = Z_TSTOBJ_P(id)))
        RETURN_BOOL(otic_pack_closeChannel(intern->oticPack, channelId));
}

PHP_METHOD(OticPack, __destruct)
{
    zval* id = getThis();
    oticPack_object* intern = Z_TSTOBJ_P(id);
    if (!intern)
        return;
    otic_pack_close(intern->oticPack);
}

zend_object* oticPack_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticPack_object* intern = (oticPack_object*)ecalloc(1, sizeof(oticPack_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    intern->std.handlers = &oticPack_handlers;
    return &intern->std;
}

static void oticPack_object_destroy(zend_object* object)
{
    oticPack_object* my_obj = (oticPack_object*)((char*)object - XtOffsetOf(oticPack_object, std));
    zend_objects_destroy_object(object);
}

static void oticPack_object_free(zend_object* object)
{
    oticPack_object* my_obj = (oticPack_object*)((char*)object - XtOffsetOf(oticPack_object, std));
    efree(my_obj->oticPack);
    zend_object_std_dtor(object);
}

const zend_function_entry oticPack_methods[] = {
        PHP_ME(OticPack, __construct, FUNCTION_PARAM, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticPack, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(OticPack, flush, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, close, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, defineChannel, argInfo_defineChannel, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, closeChannel, argInfo_closeChannel, ZEND_ACC_PUBLIC)
        PHP_FE_END
};


uint8_t oticPackStream_flusher(uint8_t* content, size_t size, void* data)
{
    php_stream_write((php_stream*)data, (const char*)content, size);
    return 1;
}

PHP_METHOD(OticPackStream, __construct)
{
    zval* id = getThis();
    zval* fileOut;
    php_stream* stream;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &fileOut) == FAILURE)
        RETURN_FALSE
    oticPack_object* intern = Z_TSTOBJ_P(id);
    if (intern) {
        php_stream_from_zval(stream, fileOut);                  // Error on PHP's website
        intern->oticPack = emalloc(sizeof(otic_pack_t));
        if (!otic_pack_init(intern->oticPack, oticPackStream_flusher, stream))
            RETURN_FALSE
    }
}

PHP_METHOD(OticPackStream, __toString)
{
    RETURN_STRING(ZEND_NS_NAME("Otic", "OticPackStream"))
}

PHP_METHOD(OticPackStream, defineChannel)
{
    zval* id = getThis();
    long channelId;
    zend_string* channelType;
    long channelFeatures;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lSl", &channelId, &channelType, &channelFeatures) == FAILURE)
        RETURN_FALSE
    oticPack_object* intern = Z_TSTOBJ_P(id);
    if (!intern)
        RETURN_FALSE
    channel_type_e type;
    if (zend_string_equals_literal(channelType, "sensor"))
        type = OTIC_CHANNEL_TYPE_SENSOR;
    else if (zend_string_equals_literal(channelType, "binary"))
        type = OTIC_CHANNEL_TYPE_BINARY;
    else
        RETURN_FALSE
    zend_string_release(channelType);
    oticPackChannel_object* channel = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(oticPackChannel_ce));
    zend_object_std_init(&channel->std, oticPackChannel_ce TSRMLS_CC);
    channel->std.handlers = &oticPackChannel_handlers;
    channel->oticPackChannel = otic_pack_defineChannel(intern->oticPack, type, channelId, 0x00);
    RETURN_OBJ(&channel->std)
}

PHP_METHOD(OticPackStream, __destruct)
{
    zval* id = getThis();
    oticPack_object* intern = Z_TSTOBJ_P(id);
    if (intern)
        otic_pack_close(intern->oticPack);
}

const zend_function_entry oticPackStream_methods[] = {
        PHP_ME(OticPackStream, __construct, OTIC_STREAM_SINGLE_ARG, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticPackStream, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackStream, defineChannel, argInfo_defineChannel, ZEND_ACC_PUBLIC)
        PHP_ME(OticPackStream, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_FE_END
};

zend_object* oticPackStream_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticPack_object* intern = (oticPack_object*)ecalloc(1, sizeof(oticPack_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &oticPackStream_handlers;
    return &intern->std;
}

static void oticPackStream_object_destroy(zend_object* object)
{
    oticPack_object* my_obj = (oticPack_object*)((char*)object - XtOffsetOf(oticPack_object, std));
    zend_objects_destroy_object(object);
}

static void oticPackStream_object_free(zend_object* object)
{
    oticPack_object* my_obj = (oticPack_object*)((char*)object - XtOffsetOf(oticPack_object, std));
    efree(my_obj->oticPack);
    zend_object_std_dtor(object);
}







// TODO: Change
zend_fcall_info pfci;
zend_fcall_info_cache pfcc;

// TODO: Can be improved ... pointless overhead!
uint8_t otic_fetcher(uint8_t* content, size_t size, void* file)
{
    zval ret, params;
    pfci.retval = &ret;
    pfci.params = &params;
    pfci.param_count = 1;
    ZVAL_LONG(&params, size)
    if (zend_call_function(&pfci, &pfcc) == FAILURE)
        return 0;
    memcpy(content, Z_STRVAL_P(&ret), Z_STRLEN(ret));
    return 1;
}

PHP_METHOD(OticUnpack, __construct) {
    zval *id = getThis();
    oticUnpack_object *intern = Z_OUPOBJ_P(id);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &pfci, &pfcc) == FAILURE)
        RETURN_FALSE
//    ZVAL_OBJ(&ret, fci.object);
//    zend_update_property(oticPackChannel_ce, id, "fetcher", strlen("fetcher"),)
    if (intern) {
        intern->oticUnpack = emalloc(sizeof(otic_unpack_t));
//        if (otic_unpack_init(intern->oticUnpack, otic_fetcher, 0))
//            RETURN_TRUE
    }
    zend_throw_exception(zend_ce_exception, "Init failed", 0);
}

PHP_METHOD(OticUnpack, __toString)
{
    zval* id = getThis();
    oticUnpack_object* intern = Z_OUPOBJ_P(id);
    if (zend_parse_parameters_none() == FAILURE)
        return;
    if (intern)
        RETURN_STRING(ZEND_NS_NAME("Otic", "OticUnpack"))
}

// TODO: Quick and dirty! MUSS BE CHANGED
zend_fcall_info dfci;
zend_fcall_info_cache dfcc;

uint8_t otic_php_flusher(uint8_t* content, size_t size)
{
    static zval ret;
    static zval params[2];
    dfci.retval = &ret;
    return zend_call_function(&dfci, &dfcc) != FAILURE;
}

PHP_METHOD(OticUnpack, defineChannel)
{
    zval* id = getThis();
    oticUnpack_object *intern = Z_OUPOBJ_P(id);
    long channelId;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lf", &channelId, &dfci, &dfcc) == FAILURE)
        return;
//    if (intern)
//        RETURN_BOOL(otic_unpack_defineChannel(intern->oticUnpack, channelId, otic_php_flusher, ))
}

PHP_METHOD(OticUnpack, closeChannel)
{
    zval* id = getThis();
    oticUnpack_object *intern = Z_OUPOBJ_P(id);
    long channelId;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelId) == FAILURE)
        return;
    if (channelId > 0xFF)
        zend_throw_exception(zend_ce_exception, "Ids should not overflow 255", 0);
    if (intern)
        RETURN_BOOL(otic_unpack_closeChannel(intern->oticUnpack, channelId))
}

PHP_METHOD(OticUnpack, parse)
{
    zval* id = getThis();
    oticUnpack_object *intern = Z_OUPOBJ_P(id);
    if (zend_parse_parameters_none() == FAILURE)
        return;
    if (intern)
        RETURN_BOOL(otic_unpack_parse(intern->oticUnpack))
}

PHP_METHOD(OticUnpack, close)
{
    zval* id = getThis();
    oticUnpack_object *intern = Z_OUPOBJ_P(id);
    if (zend_parse_parameters_none() == FAILURE)
        return;
    if (intern)
        RETURN_BOOL(otic_unpack_close(intern->oticUnpack))
}

PHP_METHOD(OticUnpack, __destruct)
{

}

const zend_function_entry oticUnpack_methods[] = {
        PHP_ME(OticUnpack, __construct, FUNCTION_PARAM, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticUnpack, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(OticUnpack, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, defineChannel, argInfo_otic_unpack_defineChannel, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, closeChannel, argInfo_otic_unpack_closeChannel, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, parse, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, close, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_object* oticUnpack_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticUnpack_object* intern = (oticUnpack_object*)ecalloc(1, sizeof(oticUnpack_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &oticUnpack_handlers;
    return &intern->std;
}

static void oticUnpack_object_destroy(zend_object* object)
{
    oticUnpack_object* my_obj = (oticUnpack_object*)((char*)object - XtOffsetOf(oticUnpack_object, std));
    zend_objects_destroy_object(object);
}

static void oticUnpack_object_free(zend_object* object)
{
    oticUnpack_object* my_obj = (oticUnpack_object*)((char*)object - XtOffsetOf(oticUnpack_object, std));
    efree(my_obj->oticUnpack);
    zend_object_std_dtor(object);
}

uint8_t php_oticUnpackStream_fetcher(uint8_t* content, size_t size, void* data)
{
    php_stream_read((php_stream*)data, (char*)content, size);
    return 1;
}

PHP_METHOD(OticUnpackStream, __construct)
{
    zval* id = getThis();
    zval* fileIn;
    php_stream* stream;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &fileIn) == FAILURE)
        RETURN_FALSE
    oticUnpack_object* intern = Z_OUPOBJ_P(id);
    if (intern) {
        php_stream_from_zval(stream, fileIn);
        intern->oticUnpack = emalloc(sizeof(otic_unpack_t));
        if (!otic_unpack_init(intern->oticUnpack, php_oticUnpackStream_fetcher, stream, 0, 0))
            RETURN_FALSE
    }
}

PHP_METHOD(OticUnpackStream, __toString)
{
    RETURN_STRING(ZEND_NS_NAME("Otic", "OticUnpackStream"))
}

uint8_t php_oticUnpackStream_flusher(uint8_t* content, size_t size, void* data)
{
    php_stream_write((php_stream*)data, (const char*)content, size);
    return 1;
}

PHP_METHOD(OticUnpackStream, defineChannel)
{
    zval* id = getThis();
    long channelId;
    zval* fileOut;
    php_stream* stream;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lr", &channelId, &fileOut) == FAILURE)
        RETURN_FALSE
    oticUnpack_object* intern = Z_OUPOBJ_P(id);
    if (!intern)
        RETURN_FALSE
    php_stream_from_zval(stream, fileOut);
    RETURN_BOOL(otic_unpack_defineChannel(intern->oticUnpack, channelId, php_oticUnpackStream_flusher, stream));
}

PHP_METHOD(OticUnpackStream, closeChannel)
{
    long channelId;
    zval* id = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelId) == FAILURE)
        RETURN_FALSE
    oticUnpack_object *intern = Z_OUPOBJ_P(id);
    if (!intern)
        RETURN_FALSE
    RETURN_BOOL(otic_unpack_closeChannel(intern->oticUnpack, channelId))
}

PHP_METHOD(OticUnpackStream, parse)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_FALSE
    oticUnpack_object* intern = Z_OUPOBJ_P(id);
    if (!intern)
        RETURN_FALSE
    RETURN_BOOL(otic_unpack_parse(intern->oticUnpack))
}

PHP_METHOD(OticUnpackStream, __destruct)
{
    zval* id = getThis();
    oticUnpack_object* intern = Z_OUPOBJ_P(id);
    if (intern)
        otic_unpack_close(intern->oticUnpack);
}

zend_object* oticUnpackStream_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticUnpack_object* intern = (oticUnpack_object*)ecalloc(1, sizeof(oticUnpack_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    intern->std.handlers = &oticUnpackStream_handlers;
    return &intern->std;
}

static void oticUnpackStream_object_destroy(zend_object* object)
{
    oticUnpack_object* my_obj = (oticUnpack_object*)((char*)object - XtOffsetOf(oticUnpack_object, std));
    zend_objects_destroy_object(object);
}

static void oticUnpackStream_object_free(zend_object* object)
{
    oticUnpack_object* my_obj = (oticUnpack_object*)((char*)object - XtOffsetOf(oticUnpack_object, std));
    efree(my_obj->oticUnpack);
    zend_object_std_dtor(object);
}

const zend_function_entry oticUnpackStream_methods[] = {
        PHP_ME(OticUnpackStream, __construct, OTIC_STREAM_INPUT_FILE_ARG, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticUnpackStream, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(OticUnpackStream, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackStream, defineChannel, argInfo_closeChannel, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackStream, closeChannel, argInfo_closeChannel, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackStream, parse, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};


PHP_MINIT_FUNCTION(otic)
{
    zend_class_entry oticPackChannel_zce;
    INIT_CLASS_ENTRY(oticPackChannel_zce, "Otic\\OticPackChannel", oticPackChannel_methods)
    oticPackChannel_ce = zend_register_internal_class(&oticPackChannel_zce TSRMLS_CC);
    oticPackChannel_ce->create_object = oticPackChannel_object_new;
    memcpy(&oticPackChannel_handlers, zend_get_std_object_handlers(), sizeof(oticPackChannel_handlers));
    oticPackChannel_handlers.free_obj = oticPackChannel_object_free;
    oticPackChannel_handlers.dtor_obj = oticPackChannel_object_destroy;
    oticPackChannel_handlers.offset = XtOffsetOf(oticPackChannel_object, std);

    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Otic\\OticPack", oticPack_methods)
    oticPack_ce = zend_register_internal_class(&ce TSRMLS_CC);
    oticPack_ce->create_object = oticPack_object_new;
    memcpy(&oticPack_handlers, zend_get_std_object_handlers(), sizeof(oticPack_handlers));
    oticPack_handlers.free_obj = oticPack_object_free;
    oticPack_handlers.dtor_obj = oticPack_object_destroy;
    oticPack_handlers.offset = XtOffsetOf(oticPack_object, std);

    zend_class_entry oticPackStream_dce;
    INIT_CLASS_ENTRY(oticPackStream_dce, "Otic\\OticPackStream", oticPackStream_methods)
    oticPackStream_ce = zend_register_internal_class(&oticPackStream_dce TSRMLS_CC);
    oticPackStream_ce->create_object = oticPackStream_object_new;
    memcpy(&oticPackStream_handlers, zend_get_std_object_handlers(), sizeof(oticPackStream_handlers));
    oticPackStream_handlers.dtor_obj = oticPackStream_object_destroy;
    oticPackStream_handlers.offset = XtOffsetOf(oticPack_object, std);

    // Out of Intellectual Curiousity, let:
    zend_class_entry oticUnpack_zce;
    INIT_CLASS_ENTRY(oticUnpack_zce, "Otic\\OticUnpack", oticUnpack_methods)
    oticUnpack_ce = zend_register_internal_class(&oticUnpack_zce TSRMLS_CC);
    oticUnpack_ce->create_object = oticUnpack_object_new;
    memcpy(&oticUnpack_handlers, zend_get_std_object_handlers(), sizeof(oticUnpack_handlers));
    oticUnpack_handlers.free_obj = oticUnpack_object_free;
    oticUnpack_handlers.dtor_obj = oticUnpack_object_destroy;
    oticUnpack_handlers.offset = XtOffsetOf(oticUnpack_object, std);

    zend_class_entry oticUnpackStream_dce;
    INIT_CLASS_ENTRY(oticUnpackStream_dce, "Otic\\OticUnpackStream", oticUnpackStream_methods)
    oticUnpackStream_ce = zend_register_internal_class(&oticUnpackStream_dce TSRMLS_CC);
    oticUnpackStream_ce->create_object = oticUnpackStream_object_new;
    memcpy(&oticUnpackStream_handlers, zend_get_std_object_handlers(), sizeof(oticUnpackStream_handlers));
    oticUnpackStream_handlers.dtor_obj = oticUnpackStream_object_destroy;
    oticUnpackStream_handlers.offset = XtOffsetOf(oticUnpack_object, std);

    return SUCCESS;
}

zend_module_entry otic_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_OTIC_EXTNAME,
    NULL,                               // Functions
    PHP_MINIT(otic),
    NULL,                               // MSHUTDOWN
    NULL,                               // RINIT
    NULL,                               // RSHUTDOWN
    NULL,                               // MINFO
#if ZEND_MODULE_API_NO >= 20010901
    PHP_OTIC_EXTVER,
#endif
    STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(otic)