#include "otic_php_unpack.h"
#include "otic_exception.h"
#include <zend_exceptions.h>
#include <zend_interfaces.h>

#include <zend_generators.h>
#include <otic.h>
#include <core/base.h>

/**
 * Useful Links:
 *
 * For anything class-related:
 * http://www.phpinternalsbook.com/php5/classes_objects/simple_classes.html
 * https://wiki.php.net/internals/engine/objects
 *
 * Parsing function, saving it and calling it
 * https://github.com/190235047/swoole-src/blob/master/swoole_process.c
 *
 * Array handling
 * https://www.php.net/manual/de/internals2.ze1.zendapi.php
 *
 * Variadic functions:
 * https://github.com/ryantenney/php7/blob/master/README.PARAMETER_PARSING_API
 * https://wiki.php.net/phpng-upgrading
 *
 * Zend Streams:
 * https://www.php.net/manual/de/internals2.ze1.streams.php
 */


zend_object_handlers oticUnpackChannel_object_handlers;
zend_class_entry* oticUnpackChannel_ce;

#define T_OTICUNPACKOBJ_P(zv) php_oticUnpack_obj_from_obj(Z_OBJ_P((zv)))
#define Z_OUNPACKCHAN(zv) php_oticUnpackChannel_obj_from_obj(Z_OBJ_P((zv)))
#define TO_STRING_AND_LENGTH(s) s, strlen(s)

static inline oticUnpack_object* php_oticUnpack_obj_from_obj(zend_object* obj)
{
    return (oticUnpack_object*)((char*)(obj) - XtOffsetOf(oticUnpack_object, std));
}

static inline oticUnpackChannel_object* php_oticUnpackChannel_obj_from_obj(zend_object* obj)
{
    return (oticUnpackChannel_object *) ((char *) (obj) - XtOffsetOf(oticUnpackChannel_object, std));
}

static inline uint8_t otic_zend_stream_isOpened(php_stream* stream)
{
    return stream->orig_path != 0;
}

PHP_METHOD(OticUnpackChannel, __construct)
{
    zval* callback;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) == FAILURE)
       return;
    if (!zend_is_callable(callback, 0, 0))
        otic_php_throw_oticException("Invalid Callable", -4);
    zval* id = getThis();
    oticUnpackChannel_object* intern = Z_OUNPACKCHAN(id);
    if (!intern)
        return;
    zend_update_property(oticUnpackChannel_ce, id, TO_STRING_AND_LENGTH("callback"), callback);
}

PHP_METHOD(OticUnpackChannel, __destruct)
{
}

PHP_METHOD(OticUnpackChannel, getTimeInterval)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticUnpackChannel_object* intern = Z_OUNPACKCHAN(id);
    if (!intern || !intern->oticUnpackChannel)
        return;
    array_init(return_value);
    add_index_double(return_value, 0, intern->oticUnpackChannel->timeInterval.time_start);
    add_index_double(return_value, 1, intern->oticUnpackChannel->ts);
}

PHP_METHOD(OticUnpackChannel, getSensorsList)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticUnpackChannel_object* intern = Z_OUNPACKCHAN(id);
    if (!intern || !intern->oticUnpackChannel)
        return;
    array_init(return_value);
    size_t counter;
    for (counter = 0; counter < intern->oticUnpackChannel->cache.totalEntries; ++counter)
        add_next_index_string(return_value, intern->oticUnpackChannel->cache.cache[counter]->name);
}

PHP_METHOD(OticUnpackChannel, __toString)
{
    zval* id = getThis();
    oticUnpackChannel_object* intern = Z_OUNPACKCHAN(id);
    if (!intern || !intern->oticUnpackChannel) {
        return;
    }
    char buffer[512] = {};
    sprintf(buffer, "<Class: %s. Id: %hhu. Type: %hhu. Total Entries: %lu. Ts: %lf - %lf>",
            ZEND_NS_NAME("Otic", "OticUnpackChannel"), intern->oticUnpackChannel->info.channelId,
            intern->oticUnpackChannel->info.channelType, intern->oticUnpackChannel->cache.totalEntries,
            intern->oticUnpackChannel->timeInterval.time_start == TS_NULL ? 0 : intern->oticUnpackChannel->timeInterval.time_start,
            intern->oticUnpackChannel->ts == TS_NULL ? 0: intern->oticUnpackChannel->ts);
    RETURN_STRING(buffer);
}

PHP_METHOD(OticUnpackChannel, setFetchList)
{
    zval* id = getThis();
    zval* argv = 0;
    int argn = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &argv, &argn) == FAILURE)
        return;
    const char* buffer[argn];
    for (int counter = 0; counter < argn; ++counter)
    {
        if (Z_TYPE(argv[counter]) != IS_STRING)
            zend_throw_exception(oticExceptions_ce, "Expected string!", -4);
        buffer[counter] = Z_STRVAL(argv[counter]);
    }
    oticUnpackChannel_object* intern = Z_OUNPACKCHAN(id);
    if (!intern || !intern->oticUnpackChannel)
        return;
    otic_unpack_channel_toFetch(intern->oticUnpackChannel, buffer, argn);
}

PHP_METHOD(OticUnpackChannel, generate)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticUnpackChannel_object *intern = Z_OUNPACKCHAN(getThis());
    if (!intern || !intern->oticUnpackChannel)
        return;
}

PHP_METHOD(OticUnpackChannel, close)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticUnpackChannel_object *intern = Z_OUNPACKCHAN(getThis());
    if (!intern || !intern->oticUnpackChannel)
        return;
    if (!otic_unpack_channel_close(intern->oticUnpackChannel))
        otic_php_throw_libOticException(intern->oticUnpackChannel->base.error);
}

ZEND_BEGIN_ARG_INFO(ARGINFO_SETFETCHLIST, 0)
    ZEND_ARG_VARIADIC_INFO(0, "sensorNames")
ZEND_END_ARG_INFO()

const zend_function_entry oticUnpackChannel_methods[] = {
        PHP_ME(OticUnpackChannel, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticUnpackChannel, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(OticUnpackChannel, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackChannel, setFetchList, ARGINFO_SETFETCHLIST, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackChannel, getTimeInterval, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackChannel, getSensorsList, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpackChannel, close, NULL, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_object* oticUnpackChannel_object_new(zend_class_entry* ce TSRMLS_DC)
{
    oticUnpackChannel_object* intern = (oticUnpackChannel_object*)ecalloc(1, sizeof(oticUnpackChannel_object) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &oticUnpackChannel_object_handlers;
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
    return php_stream_read((php_stream*)data, (char*)content, size) != 0;
}


zend_class_entry* oticUnpack_ce;
zend_object_handlers oticUnpack_object_handlers;

static inline uint8_t otic_php_unpack_seeker(uint32_t pos, void* data)
{
    return !php_stream_seek((php_stream*)data, pos, SEEK_CUR);
}

PHP_METHOD(OticUnpack, __construct)
{
    zval* fileIn;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &fileIn) == FAILURE)
        return;
    zval* id = getThis();
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    php_stream* stream;
    php_stream_from_zval(stream, fileIn);
    intern->oticUnpack = emalloc(sizeof(otic_unpack_t));
    if (!otic_unpack_init(intern->oticUnpack, otic_php_unpack_fetcher, stream, otic_php_unpack_seeker, stream))
        otic_php_throw_libOticException(intern->oticUnpack->error);
}

PHP_METHOD(OticUnpack, __toString)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    char buffer[512] = {};
    sprintf(buffer, "<Class: %s. Total Channels: %hhu. State: %hhu. Error: %hhu>",
            ZEND_NS_NAME("Otic", "OticUnpack"), otic_unpack_getTotalAmountOfChannel(intern->oticUnpack), intern->oticUnpack->state,
            intern->oticUnpack->error);
    RETURN_STRING(buffer);
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
            otic_php_throw_libOticException(intern->oticUnpack->error);
}

static inline uint8_t oticUnpack_channelSelect_wrapper(double timestamp, const char* sensorName, const char* sensorUnit, const oval_t* val, void* data)
{
    zval ret;
    zval params[4];
    ZVAL_DOUBLE(&params[0], timestamp)
    ZVAL_STRING(&params[1], sensorName);
    ZVAL_STRING(&params[2], sensorUnit);
    switch (val->type)
    {
        case OTIC_TYPE_NULL:
            ZVAL_NULL(&params[3]);
            break;
        case OTIC_TYPE_INT_POS:
            ZVAL_LONG(&params[3], val->val.lval)
            break;
        case OTIC_TYPE_INT_NEG:
            ZVAL_LONG(&params[3], -val->val.lval)
            break;
        case OTIC_TYPE_DOUBLE:
            ZVAL_DOUBLE(&params[3], val->val.dval)
            break;
        case OTIC_TYPE_STRING:
            ZVAL_STRING(&params[3], val->val.sval.ptr);
            break;
        case OTIC_TYPE_TRUE:
            ZVAL_TRUE(&params[3]);
            break;
        case OTIC_TYPE_FALSE:
            ZVAL_FALSE(&params[3]);
            break;
        default:
            otic_php_throw_oticException("Unknown Type", 0);
    }
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
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    if (channelId < 0 || channelId > 255)
        otic_php_throw_oticException("Invalid Channel ID! Reason: Valid Range (int): 0 - 255", 0);
    zend_string* funcName;
    if (!zend_is_callable(callback, 0, &funcName TSRMLS_CC)) {
        php_error_docref0(NULL TSRMLS_CC, E_ERROR, "Invalid Callback %s", funcName->val);
        zend_string_release(funcName);
    }
    zend_string_release(funcName);
    oticUnpackChannel_object* channel = (oticUnpackChannel_object*)ecalloc(1, sizeof(oticUnpackChannel_object) + zend_object_properties_size(oticUnpackChannel_ce));
    zval retVal;
    ZVAL_ZVAL(&channel->funcptr, callback, 1, 1);
    channel->funcptr.value.ref->gc.refcount++;
    zend_call_method_with_1_params(id, oticUnpackChannel_ce, &oticUnpackChannel_ce->constructor, "__construct", &retVal, &channel->funcptr);
    channel->oticUnpackChannel = otic_unpack_defineChannel(intern->oticUnpack, channelId, oticUnpack_channelSelect_wrapper, &channel->funcptr);
    if (!channel->oticUnpackChannel)
    {
        efree(channel);
        otic_php_throw_libOticException(intern->oticUnpack->error);
    }
    zend_object_std_init(&channel->std, oticUnpackChannel_ce TSRMLS_CC);
    object_properties_init(&channel->std, oticUnpackChannel_ce);
    channel->std.handlers = &oticUnpack_object_handlers;
    RETURN_OBJ(&channel->std)
}

PHP_METHOD(OticUnpack, parse)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    otic_unpack_parse(intern->oticUnpack);
    if (intern->oticUnpack->error != 0)
        otic_php_throw_libOticException(intern->oticUnpack->error);
}

PHP_METHOD(OticUnpack, generate)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticUnpack_object *intern = T_OTICUNPACKOBJ_P(id);
    if (!intern || !intern->oticUnpack)
        return;
    if (!otic_unpack_generate(intern->oticUnpack))
        return;
    array_init(return_value);
    if (intern->oticUnpack->current && intern->oticUnpack->current->cache.currentEntry) {
        add_next_index_long(return_value, intern->oticUnpack->current->info.channelId);
        add_next_index_double(return_value, intern->oticUnpack->current->ts);
        add_next_index_string(return_value, intern->oticUnpack->current->cache.currentEntry->name);
        add_next_index_string(return_value, intern->oticUnpack->current->cache.currentEntry->unit);
        switch (intern->oticUnpack->current->cache.currentEntry->value.type)
        {
            case OTIC_TYPE_NULL:
                add_next_index_null(return_value);
                break;
            case OTIC_TYPE_INT_POS:
                add_next_index_long(return_value, intern->oticUnpack->current->cache.currentEntry->value.val.lval);
                break;
            case OTIC_TYPE_INT_NEG:
                add_next_index_long(return_value, -intern->oticUnpack->current->cache.currentEntry->value.val.lval);
                break;
            case OTIC_TYPE_DOUBLE:
                add_next_index_double(return_value, intern->oticUnpack->current->cache.currentEntry->value.val.dval);
                break;
            case OTIC_TYPE_STRING:
                add_next_index_string(return_value, intern->oticUnpack->current->cache.currentEntry->value.val.sval.ptr);
                break;
            case OTIC_TYPE_TRUE:
                add_next_index_bool(return_value, 1);
                break;
            case OTIC_TYPE_FALSE:
                add_next_index_bool(return_value, 0);
                break;
            case OTIC_TYPE_EOF:
                break;
            default:
                otic_php_throw_oticException("Unknown Type", 0);
        }
    }
}

PHP_METHOD(OticUnpack, read)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    while (otic_unpack_parse(intern->oticUnpack));
}

PHP_METHOD(OticUnpack, close)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    zval* id = getThis();
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    if (intern->oticUnpack->state != OTIC_STATE_CLOSED)
        if (!otic_unpack_close(intern->oticUnpack))
            otic_php_throw_libOticException(intern->oticUnpack->error);
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
        PHP_ME(OticUnpack, read, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, generate, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, close, NULL, ZEND_ACC_PUBLIC)
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

#undef TO_STRING_AND_LENGTH
#undef T_OTICUNPACKOBJ_P
#undef Z_OUNPACKCHAN