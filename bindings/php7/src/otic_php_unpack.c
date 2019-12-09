//
// Created by talpaadmin on 06.12.19.
//

#include "otic_php_unpack.h"
#include "oticException.h"
#include <zend_exceptions.h>

// TODO:
#include <zend_generators.h>
#include <core/base.h>

/**
 * Useful Links:
 *
 *
 * For anything class-related:
 * http://www.phpinternalsbook.com/php5/classes_objects/simple_classes.html
 * https://wiki.php.net/internals/engine/objects
 *
 *
 * Parsing function, saving it and calling it
 * https://github.com/190235047/swoole-src/blob/master/swoole_process.c
 *
 *
 * Array handling
 * https://www.php.net/manual/de/internals2.ze1.zendapi.php
 *
 *
 * Variadic functions:
 * https://github.com/ryantenney/php7/blob/master/README.PARAMETER_PARSING_API
 * https://wiki.php.net/phpng-upgrading
 */


zend_object_handlers oticUnpackChannel_object_handlers;

zend_class_entry* oticUnpackChannel_ce;

static inline oticUnpack_object* php_oticUnpack_obj_from_obj(zend_object* obj)
{
    return (oticUnpack_object*)((char*)(obj) - XtOffsetOf(oticUnpack_object, std));
}

#define T_OTICUNPACKOBJ_P(zv) php_oticUnpack_obj_from_obj(Z_OBJ_P((zv)))

static inline oticUnpackChannel_object* php_oticUnpackChannel_obj_from_obj(zend_object* obj)
{
    return (oticUnpackChannel_object *) ((char *) (obj) - XtOffsetOf(oticUnpackChannel_object, std));
}

#define Z_OUNPACKCHAN(zv) php_oticUnpackChannel_obj_from_obj(Z_OBJ_P((zv)))


PHP_METHOD(OticUnpackChannel, __toString)
{
    RETURN_STRING(ZEND_NS_NAME("Otic", "OticUnpackChannel"))
}

PHP_METHOD(OticUnpackChannel, setFetchList)
{
    zval* id = getThis();
    zval* argv = 0;
    int argn = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &argv, &argn) == FAILURE)
        return;
    const char* buffer[argn];

    // TODO: Check if all inputs are strings
    for (int counter = 0; counter < argn; ++counter)
        buffer[counter] = Z_STRVAL(argv[counter]);
    oticUnpackChannel_object* intern = Z_OUNPACKCHAN(id);
    if (!intern)
        return;
    otic_unpack_channel_toFetch(intern->oticUnpackChannel, buffer, 2);
}

PHP_METHOD(OticUnpackChannel, close)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticUnpackChannel_object *intern = Z_OUNPACKCHAN(getThis());
    if (!intern)
        return;
    if (!otic_unpack_channel_close(intern->oticUnpackChannel))
        zend_throw_exception(oticExceptions_ce, otic_strError(intern->oticUnpackChannel->base.error), intern->oticUnpackChannel->base.error);
}

ZEND_BEGIN_ARG_INFO(ARGINFO_SETFETCHLIST, 0)
    ZEND_ARG_VARIADIC_INFO(0, "sensorNames")
ZEND_END_ARG_INFO()

const zend_function_entry oticUnpackChannel_methods[] = {
        PHP_ME(OticUnpackChannel, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackChannel, setFetchList, ARGINFO_SETFETCHLIST, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackChannel, close, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_object* oticUnpackChannel_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticUnpackChannel_object* intern = (oticUnpackChannel_object*)ecalloc(1, sizeof(oticUnpackChannel_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &oticUnpack_object_handlers;
    return &intern->std;
}

void oticUnpackChannel_object_destroy(zend_object* object)
{
    oticUnpackChannel_object* myObj = (oticUnpackChannel_object*)((char*)object - XtOffsetOf(oticUnpackChannel_object, std));
    zend_objects_destroy_object(object);
}

void oticUnpackChannel_object_free(zend_object* object)
{
    oticUnpackChannel_object* myObj = (oticUnpackChannel_object*)((char*)object - XtOffsetOf(oticUnpackChannel_object, std));
    zend_object_std_dtor(object);
}

static inline uint8_t otic_php_unpack_fetcher(uint8_t* content, size_t size, void* data)
{
    return php_stream_read((php_stream*)data, (char*)content, size);
}


zend_class_entry* oticUnpack_ce;
zend_object_handlers oticUnpack_object_handlers;

static inline uint8_t otic_php_unpack_seeker(uint32_t pos, void* data)
{
    return !php_stream_seek((php_stream*)data, SEEK_CUR, pos);
}

PHP_METHOD(OticUnpack, __construct)
{
    zval* id = getThis();
    php_stream* stream;
    zval* fileIn;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &fileIn) == FAILURE)
        return;
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    php_stream_from_zval(stream, fileIn);
    intern->oticUnpack = emalloc(sizeof(otic_unpack_t));
    if (!otic_unpack_init(intern->oticUnpack, otic_php_unpack_fetcher, stream, otic_php_unpack_seeker, stream))
        zend_throw_exception(libOticExceptions_ce, "", intern->oticUnpack->error);
    zend_declare_property_null(oticUnpack_ce, "flusher", strlen("flusher"), ZEND_ACC_PROTECTED);
    // TODO:
//    zend_declare_class_constant()
}

PHP_METHOD(OticUnpack, __toString)
{
    RETURN_STRING(ZEND_NS_NAME("otic", "OticUnpack"))
}

PHP_METHOD(OticUnpack, __destruct)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    if (intern->oticUnpack->state != OTIC_STATE_CLOSED)
        if (!otic_unpack_close(intern->oticUnpack))
            zend_throw_exception(libOticExceptions_ce, "", intern->oticUnpack->error);
}

static inline uint8_t oticUnpack_channelSelect_wrapper(double timestamp, const char* sensorName, const char* sensorUnit, const oval_t* val, void* data)
{
    zval ret;
    zval params[4];
    ZVAL_DOUBLE(&params[0], timestamp)
    ZVAL_STRING(&params[1], sensorName);
    ZVAL_STRING(&params[2], sensorUnit);
    ZVAL_NULL(&params[3]);
    switch (val->type)
    {
        case OTIC_TYPE_NULL:
            ZVAL_NULL(&params[3]);
            break;
        case OTIC_TYPE_INT32_POS:
            ZVAL_LONG(&params[3], val->lval.value)
            break;
        case OTIC_TYPE_INT32_NEG:
            ZVAL_LONG(&params[3], -val->lval.value)
            break;
        case OTIC_TYPE_DOUBLE:
            ZVAL_DOUBLE(&params[3], val->dval)
            break;
        case OTIC_TYPE_STRING:
            ZVAL_STRING(&params[3], val->sval.ptr);
            break;
        default:
            zend_throw_exception(oticExceptions_ce, "Unknown Type", 0);
    }
//    zval* func = zend_read_property(oticUnpack_ce, data, "flusher", strlen("flusher"), 1, 0);
//    if (zend_is_callable((zval*)data, 0, 0))
//        php_printf("Uncallable\n");
    if (call_user_function_ex(EG(function_table), NULL, data, &ret, 4, params, 0, 0 TSRMLS_CC) == FAILURE)
        php_error_docref0(NULL TSRMLS_CC, E_WARNING, "User Function Error\n");
    return 1;
}

PHP_METHOD(OticUnpack, selectChannel)
{
    zval* id =  getThis();
    zval* callback;
    long channelId;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &channelId, &callback) == FAILURE)
        return;
    if (channelId < 0 || channelId > 255)
        zend_throw_exception(oticExceptions_ce, "Invalid Id", 0);
    if (!zend_is_callable(callback, 0, 0))
        zend_throw_exception(oticExceptions_ce, "Invalid Callback", 0);
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;


//  TODO: Find a way to insert callback into class
//  zend_update_property(oticUnpack_ce, getThis(), "flusher", strlen("flusher"), callback TSRMLS_CC);

//    zval retVal;
//    call_user_function_ex(EG(function_table), 0, ret, &retVal, 0, 0, 0, 0);
//    if (zend_is_callable(&channel->funcptr, 0, 0)) {
//        php_printf("Is callable\n");
//        zval retVal;
//        call_user_function(EG(function_table), 0, callback, &retVal, 0, 0);
//    }
//    zval* ret = zend_read_property(oticUnpack_ce, id, "flusher", strlen("flusher"), 1, 0);


    oticUnpackChannel_object* channel = (oticUnpackChannel_object*)ecalloc(1, sizeof(oticUnpackChannel_object) + zend_object_properties_size(oticUnpackChannel_ce));
    ZVAL_ZVAL(&channel->funcptr, callback, 0, 0);
    zend_object_std_init(&channel->std, oticUnpackChannel_ce TSRMLS_CC);
    channel->std.handlers = &oticUnpack_object_handlers;

    channel->oticUnpackChannel = otic_unpack_defineChannel(intern->oticUnpack, channelId, oticUnpack_channelSelect_wrapper, &channel->funcptr);
    if (channel->oticUnpackChannel->base.error != 0) {
        zend_throw_exception(oticExceptions_ce, otic_strError(channel->oticUnpackChannel->base.error), channel->oticUnpackChannel->base.error);
    }
    RETURN_OBJ(&channel->std)
}

PHP_METHOD(OticUnpack, parse)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    otic_unpack_parse(intern->oticUnpack);
    if (intern->oticUnpack->error != 0)
        zend_throw_exception(libOticExceptions_ce, otic_strError(intern->oticUnpack->error), intern->oticUnpack->error);
}


ZEND_BEGIN_ARG_INFO_EX(ARGINFO_UNPACK_CONST, 0, 0, 1)
                ZEND_ARG_INFO(0, inputFile)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ARGINFO_UNPACK_SELCHANN, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, channelId, IS_LONG, 0)
    ZEND_ARG_CALLABLE_INFO(0, flusher, 0)
ZEND_END_ARG_INFO()

const zend_function_entry oticUnpack_methods[] = {
        PHP_ME(OticUnpack, __construct, ARGINFO_UNPACK_CONST, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticUnpack, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(OticUnpack, selectChannel, ARGINFO_UNPACK_SELCHANN, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, parse, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_object* oticUnpack_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticUnpack_object* intern = (oticUnpack_object*)ecalloc(1, sizeof(oticUnpack_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &oticUnpack_object_handlers;
    return &intern->std;
}

void oticUnpack_object_destroy(zend_object* object)
{
    oticUnpack_object* myObj = (oticUnpack_object*)((char*)object - XtOffsetOf(oticUnpack_object, std));
    zend_objects_destroy_object(object);
}

void oticUnpack_object_free(zend_object* object)
{
    oticUnpack_object* myObj = (oticUnpack_object*)((char*)object - XtOffsetOf(oticUnpack_object, std));
    zend_object_std_dtor(object);
}