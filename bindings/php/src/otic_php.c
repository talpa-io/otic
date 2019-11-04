#ifdef HAVE_TIMELIB_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <otic.h>
#include <Zend/zend.h>
#include <Zend/zend_exceptions.h>

#define PHP_OTIC_EXTVER "1.0"
#define PHP_OTIC_EXTNAME "otic"

zend_object_handlers oticPack_handlers;
typedef struct
{
    otic_pack_t* oticPackBase;
    zend_object std;
} oticPack_object;

static inline oticPack_object* php_oticPack_obj_from_obj(zend_object* obj)
{
    return (oticPack_object*)((char*)(obj) - XtOffsetOf(oticPack_object, std));
}

#define Z_TSTOBJ_P(zv) php_oticPack_obj_from_obj(Z_OBJ_P((zv)))

zend_class_entry* oticPack_ce;

PHP_METHOD(OticPack, __construct)
{
    zval* id = getThis();
    oticPack_object* intern = Z_TSTOBJ_P(id);
    if (intern)
        intern->oticPackBase = emalloc(sizeof(otic_pack_t));


}

PHP_METHOD(OticPack, close)
{
    zval* id = getThis();
    oticPack_object* intern;
//    if (zend_parse_parameters_none() == FAILURE)
//        RETURN_NULL()
//    intern = Z_TSTOBJ_P(id);
//    if (intern){
//        otic_pack_close(intern->oticPackBase);
//    }

    php_printf("%s\n", __PRETTY_FUNCTION__);
    if (zend_parse_parameters_none() != FAILURE)
        RETURN_NULL()
    intern = Z_TSTOBJ_P(id);

    zval call_func_name, call_func_ret, func_params[2];
    uint32_t param_count = 2;
    ZVAL_STRING(&call_func_name, "my_sum");
    ZVAL_LONG(&func_params[9], 10)
    ZVAL_LONG(&func_params[1], 20)

    if (SUCCESS != call_user_function(EG(function_table), NULL, &call_func_name, &call_func_ret, param_count, func_params))
        RETURN_FALSE
    RETURN_LONG(Z_LVAL(call_func_ret))
}

PHP_METHOD(OticPack, test)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    oticPack_object* intern;
    zend_fcall_info fcallInfo;
    zend_fcall_info_cache fcallInfoCache;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &fcallInfo, &fcallInfoCache) == FAILURE)
        RETURN_FALSE

    php_printf("%ld\n", fcallInfo.param_count);

//    if (SUCCESS != zend_call_function(&(fcallInfo), &fcallInfoCache))
//        RETURN_LONG(-2)
    printf("Reached");
    /*php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    oticPack_object* intern;
    zval call_func_name, call_func_ret, func_params[2];
    uint32_t param_count = 2;
    ZVAL_STRING(&call_func_name, "my_sum");
    ZVAL_LONG(&func_params[0], 10)
    ZVAL_LONG(&func_params[1], 20)
    if (SUCCESS != call_user_function(EG(function_table), NULL, &call_func_name, &call_func_ret, param_count, func_params))
    {
        RETURN_LONG(-1)
    }
    php_printf("%s\n", "Reached");
    RETURN_LONG(Z_LVAL(call_func_ret))*/
}

PHP_METHOD(OticPack, flush)
{
    zval* id = getThis();
    oticPack_object* intern;
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_NULL()
    intern = Z_TSTOBJ_P(id);
    if (intern) {
        otic_pack_close(intern->oticPackBase);
    }
}

PHP_METHOD(OticPack, defineChannel)
{
    zval* id = getThis();
    oticPack_object* intern;
    long channelId;
    zend_string* channelType;
    long channelOptions;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lSl", &channelId, &channelType, &channelOptions) == FAILURE)
        return;
    php_printf("Channel ID: %ld, ChannelType: %s, ChannelOptions %ld\n", channelId, ZSTR_VAL(channelType), channelOptions);
    zend_string_release(channelType);
}

PHP_METHOD(OticPack, closeChannel)
{
    zval* id = getThis();
    oticPack_object* intern;
    long channelId;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channelId) == FAILURE)
        return;
    php_printf("ChannelId: %ld, %s\n", channelId, __PRETTY_FUNCTION__);
}

PHP_METHOD(OticPack, __destruct)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
}

ZEND_BEGIN_ARG_INFO_EX(argInfo_defineChannel, 0, 0, 3)
    ZEND_ARG_INFO(0, channelId)
    ZEND_ARG_INFO(0, channelType)
    ZEND_ARG_INFO(0, channelOptions)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(argInfo_closeChannel, 0, 0, 1)
    ZEND_ARG_INFO(0, channelId)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(FUNCTION_PARAM, 0, 0, 1)
    ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()


const zend_function_entry oticPack_methods[] = {
    PHP_ME(OticPack, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(OticPack, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    PHP_ME(OticPack, flush, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OticPack, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OticPack, test, FUNCTION_PARAM, ZEND_ACC_PUBLIC)
    PHP_ME(OticPack, defineChannel, argInfo_defineChannel, ZEND_ACC_PUBLIC)
    PHP_ME(OticPack, closeChannel, argInfo_closeChannel, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

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
    efree(my_obj->oticPackBase);
    zend_object_std_dtor(object);
}

PHP_MINIT_FUNCTION(otic)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Otic\\OticPack", oticPack_methods);
    oticPack_ce = zend_register_internal_class(&ce TSRMLS_CC);
    oticPack_ce->create_object = oticPack_object_new;

    memcpy(&oticPack_handlers, zend_get_std_object_handlers(), sizeof(oticPack_handlers));
    oticPack_handlers.free_obj = oticPack_object_free;
    oticPack_handlers.dtor_obj = oticPack_object_destroy;
    oticPack_handlers.offset = XtOffsetOf(oticPack_object, std);
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

/*
#ifdef HAVE_TIMELIB_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend.h>
#include "../../../include/config/config.h"
#include "../../../src/pack/pack.h"
#include "../../../src/unpack/unpack.h"
#define OTIC_PHP_VERSION "1.0"
#define OTIC_PHP_EXTNAME "otic"


zend_object_handlers otic_pack_handlers;
typedef struct
{
    otic_pack_base_t* oticPackBase;
    zend_object std;
} otic_pack_php_t;

static inline otic_pack_php_t* otic_pack_obj_from_obj(zend_object* obj)
{
    return (otic_pack_php_t*)((char*)(obj) - XtOffsetOf(otic_pack_php_t, std));
}

#define Z_TSTOBJ_P(zv) otic_pack_obj_from_obj(Z_OBJ_P((zv)))

static zend_class_entry* otic_pack_ce;


static FILE *file;
static uint8_t flusher(uint8_t* val, size_t size)
{
    fwrite(val, 1, size, file);
}

PHP_METHOD(OticPack, __construct)
{
    zend_string* metadata;
    zend_string* fileName;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SS", &fileName, &metadata) == FAILURE)
        return;
    file = fopen(ZSTR_VAL(fileName), "wb");
    php_printf("Metadata: %s\n", ZSTR_VAL(metadata));
    zval* obj = getThis();
    otic_pack_php_t* intern;
    intern = Z_TSTOBJ_P(obj);
    if (intern != NULL)
        intern->oticPackBase = emalloc(sizeof(otic_pack_base_t));
    otic_pack_base_init(intern->oticPackBase);
    php_printf("Constructor\n");
    zend_string_release(metadata);
    zend_string_release(fileName);
}

PHP_METHOD(OticPack, write)
{
    typedef struct {
        char* content;
        uint32_t size;
    } string_php;
    string_php sensorName;
    string_php sensorUnit;
    double timestamp = 0;
    zval* value;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dssz", &timestamp, &sensorName.content, &sensorName.size, &sensorUnit.content, &sensorUnit.size, &value) == FAILURE)
        return;
    zval* id = getThis();
    otic_pack_php_t* intern = Z_TSTOBJ_P(id);
//    if (intern == NULL)
//        return;
//    if (Z_TYPE_P(value) == IS_DOUBLE)
//        otic_pack_inject_d(intern->oticPack, timestamp, sensorName.content, sensorUnit.content, Z_DVAL_P(value));
//    else if (Z_TYPE_P(value) == IS_LONG)
//        otic_pack_inject_i(intern->oticPack, timestamp, sensorName.content, sensorUnit.content, Z_LVAL_P(value));
//    else if (Z_TYPE_P(value) == IS_STRING)
//        otic_pack_inject_s(intern->oticPack, timestamp, sensorName.content, sensorUnit.content, Z_STRVAL_P(value));
//    else if (Z_TYPE_P(value) == IS_NULL)
//        otic_pack_inject_n(intern->oticPack, timestamp, sensorName.content, sensorUnit.content);
//    else {
//        zend_throw_error(zend_ce_exception, "Invalid Type", 0);
//        return;
//    }
    php_printf("%f %s %s\n", timestamp, sensorName.content, sensorUnit.content);
}

PHP_METHOD(OticPack, writeBuffer)
{
    typedef struct {
        char* content;
        uint32_t size;
    } string_php;
    string_php _sensorName;
    string_php _sensorUnit;
    string_php _buffer;
    double timestamp = 0;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dsss", &timestamp, &_sensorName.content, &_sensorName.size, &_sensorUnit.content, &_sensorUnit.size, &_buffer.content, &_buffer.size) == FAILURE)
        return;
    zval* id = getThis();
    otic_pack_php_t* intern = Z_TSTOBJ_P(id);
    if (intern == NULL)
        return;
//    otic_pack_inject_b(intern->oticPack, timestamp, _sensorName.content, _sensorUnit.content, (uint8_t*)_buffer.content, _buffer.size);
}

PHP_METHOD(OticPack, dump)
{
    if (zend_parse_parameters_none() == FAILURE){
        return;
    }
    zval* id = getThis();
    otic_pack_php_t* intern = Z_TSTOBJ_P(id);

//    RETURN_BOOL(otic_pack_flush(intern->oticPack))
}

PHP_METHOD(OticPack, getTsIntervall)
{
    if (zend_parse_parameters_none() == FAILURE){
        return;
    }
    zval* id = getThis();
    otic_pack_php_t* intern = Z_TSTOBJ_P(id);
    array_init(return_value);
    if (intern->oticPack != 0)
    {
        add_index_double(return_value, 0, (double)intern->oticPack->base.timestamp_start / OTIC_TS_MULTIPLICATOR);
        add_index_double(return_value, 1, (double)intern->oticPack->base.timestamp_current / OTIC_TS_MULTIPLICATOR);
    }
}

PHP_METHOD(OticPack, __destruct)
{
    zval* id;
    otic_pack_php_t* intern;
    if (zend_parse_parameters_none() == FAILURE)
        return;
    id = getThis();
    intern = Z_TSTOBJ_P(id);
    if (intern->oticPack)
        otic_pack_close(intern->oticPack);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_set, 0, 0, 2)
    ZEND_ARG_INFO(0, fileName)
    ZEND_ARG_INFO(0, metadata)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_write, 0, 0, 4)
    ZEND_ARG_INFO(0, timestamp)
    ZEND_ARG_INFO(0, sensorName)
    ZEND_ARG_INFO(0, sensorUnit)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

const zend_function_entry otic_functions[] = {
        PHP_ME(OticPack, __construct, arginfo_set, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticPack, write, arginfo_write, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, writeBuffer, arginfo_write, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, dump, arginfo_void, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, getTsIntervall, arginfo_void, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, __destruct, arginfo_void, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_FE_END
};

zend_object* oticPack_new(zend_class_entry *ce TSRMLS_DC)
{
    otic_pack_php_t* intern = (otic_pack_php_t*)ecalloc(1, sizeof(otic_pack_php_t) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &otic_pack_handlers;
    return &intern->std;
}

static void oticPack_destroy(zend_object* object)
{
    otic_pack_php_t* my_obj = (otic_pack_php_t*)((char*)object - XtOffsetOf(otic_pack_php_t, std));
    zend_objects_destroy_object(object);
}

static void oticPack_free(zend_object* object)
{
    otic_pack_php_t* my_obj = (otic_pack_php_t*)((char*)object - XtOffsetOf(otic_pack_php_t, std));
    if (!my_obj)
        return;
    zend_object_std_dtor(&my_obj->std);
    efree(my_obj->oticPack);
}

zend_object_handlers oticUnpackHandlers;
typedef struct
{
    otic_unpack_t* oticUnpack;
    zend_object std;
} otic_unpack_php_t;


static zend_class_entry* otic_unpack_ce;

static inline otic_unpack_php_t* php_otic_unpack_obj_from_obj(zend_object *obj) {
    return (otic_unpack_php_t*)((char*)(obj) - XtOffsetOf(otic_unpack_php_t, std));
}

#define Z_TSTOBJ_P2(zv)  php_otic_unpack_obj_from_obj(Z_OBJ_P((zv)))

static FILE* unpack_fileIn = 0;
static FILE* unpack_fileOut = 0;

static uint8_t unpack_fetch(uint8_t* content, size_t size)
{
    fread(content, 1, size, unpack_fileIn);
    return 1;
}

static uint8_t unpack_flush(uint8_t* content, size_t size)
{
    fread(content, 1, size, unpack_fileOut);
    return 1;
}

PHP_METHOD(OticUnpack, __construct)
{
    zval* id = getThis();
    otic_unpack_php_t* intern;
    zend_string* fileIn;
    zend_string* fileOut;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "PP", &fileIn, &fileOut) == FAILURE)
    {
        RETURN_NULL()
    }
    if (!fopen(ZSTR_VAL(fileIn), "rb") || !fopen(ZSTR_VAL(fileOut), "wb"))
        RETURN_NULL()
    intern = Z_TSTOBJ_P2(id);
    if (intern)
        intern->oticUnpack = emalloc(sizeof(otic_unpack_t));
}


PHP_METHOD(OticUnpack, read)
{
    zval* id = getThis();
    otic_unpack_php_t* intern;
    if (zend_parse_parameters_none() == FAILURE){
        return;
    }
    intern = Z_TSTOBJ_P2(id);
    RETURN_BOOL(otic_unpack_parseBlock(intern->oticUnpack))
}

PHP_METHOD(OticUnpack, __destruct)
{
    zval* id = getThis();
    otic_unpack_php_t* intern;
    if (zend_parse_parameters_none() == FAILURE){
        return;
    }
    intern = Z_TSTOBJ_P2(id);
    if (intern != 0)
        otic_unpack_close(intern->oticUnpack);
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_doublePaths, 0, 0, 2)
                ZEND_ARG_INFO(0, fileinName)
                ZEND_ARG_INFO(0, fileoutName)
ZEND_END_ARG_INFO()

const zend_function_entry otic_unpack_methods[] = {
        PHP_ME(OticUnpack, __construct, arginfo_doublePaths, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticUnpack, __destruct, arginfo_void, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(OticUnpack, read, arginfo_void, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_object *otic_unpack_object_new(zend_class_entry *ce TSRMLS_DC)
{
    otic_unpack_php_t *intern = (otic_unpack_php_t*)ecalloc(1,
                                                sizeof(otic_unpack_php_t) +
                                                zend_object_properties_size(ce));

    zend_object_std_init(&intern->std, ce TSRMLS_CC);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &oticUnpackHandlers;
    return &intern->std;
}

static void otic_unpack_object_destroy(zend_object *object)
{
    otic_unpack_php_t *my_obj;
    my_obj = (otic_unpack_php_t*)((char *)
                                    object - XtOffsetOf(otic_unpack_php_t, std));
    zend_objects_destroy_object(object);
}

static void otic_unpack_object_free(zend_object *object)
{
    otic_unpack_php_t *my_obj;
    my_obj = (otic_unpack_php_t *)((char *)
                                     object - XtOffsetOf(otic_unpack_php_t, std));
    efree(my_obj->oticUnpack);
    zend_object_std_dtor(object);
}

PHP_MINIT_FUNCTION(otic)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "OticPack", otic_functions)
    otic_pack_ce = zend_register_internal_class(&ce TSRMLS_CC);
    otic_pack_ce->create_object = oticPack_new;
    memcpy(&otic_pack_handlers, zend_get_std_object_handlers(), sizeof(otic_pack_handlers));
    otic_pack_handlers.free_obj = oticPack_free;
    otic_pack_handlers.dtor_obj = oticPack_destroy;
    otic_pack_handlers.offset = XtOffsetOf(otic_pack_php_t, std);


    zend_class_entry de;
    INIT_CLASS_ENTRY(de, "OticUnpack", otic_unpack_methods)
    otic_unpack_ce = zend_register_internal_class(&de TSRMLS_CC);
    otic_unpack_ce->create_object = otic_unpack_object_new;
    memcpy(&oticUnpackHandlers, zend_get_std_object_handlers(), sizeof(oticUnpackHandlers));
    oticUnpackHandlers.free_obj = otic_unpack_object_free;
    oticUnpackHandlers.dtor_obj = otic_unpack_object_destroy;
    oticUnpackHandlers.offset = XtOffsetOf(otic_pack_php_t, std);

    return SUCCESS;
}

zend_module_entry otic_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
        STANDARD_MODULE_HEADER,
#endif
        OTIC_PHP_EXTNAME,
        otic_functions,
        PHP_MINIT(otic),
        NULL,
        NULL,
        NULL,
        NULL,
#if ZEND_MODULE_API_NO >= 20010901
        OTIC_PHP_VERSION,
#endif
        STANDARD_MODULE_PROPERTIES
};


ZEND_GET_MODULE(otic)*/
