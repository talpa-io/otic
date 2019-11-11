#ifdef HAVE_TIMELIB_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <otic.h>
#include <Zend/zend.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_types.h>

// https://wiki.php.net/phpng-upgrading

#define PHP_OTIC_EXTVER "1.0"
#define PHP_OTIC_EXTNAME "otic"

zend_object_handlers oticPack_handlers;
zend_object_handlers oticPackChannel_handlers;

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
    struct
    {
        otic_pack_t* oticPack;
        FILE* file;
        uint8_t(*flusher)(uint8_t*, size_t);
    } content;
    zend_object std;
} oticPackStream_object;

static inline oticPack_object* php_oticPack_obj_from_obj(zend_object* obj)
{
    return (oticPack_object*)((char*)(obj) - XtOffsetOf(oticPack_object, std));
}

static inline oticPackChannel_object* php_oticPackChannel_obj_from_obj(zend_object* obj)
{
    return (oticPackChannel_object*)((char*)(obj) - XtOffsetOf(oticPackChannel_object, std));
}

static inline oticPackStream_object* php_oticPackStream_obj_from_obj(zend_object* obj)
{
    return (oticPackStream_object *) ((char *) (obj) - XtOffsetOf(oticPackStream_object, std));
}

#define Z_TSTOBJ_P(zv) php_oticPack_obj_from_obj(Z_OBJ_P((zv)))
#define Z_OTICSTREAM_OBJ_P(zv) php_oticPackStream_obj_from_obj(Z_OBJ_P((zv)))
#define Z_OTICPACKCHANNELOBJ_P(zv) php_oticPackChannel_obj_from_obj(Z_OBJ_P((zv)))

zend_class_entry* oticPack_ce;
zend_class_entry* oticPackChannel_ce;
zend_class_entry* oticPackStream_ce;

// Quick and dirty
static zend_fcall_info sfci;
static zend_fcall_info_cache sfcc;
static zval sargs[2];
static zval sret;

// TODO: DESTROY zvals and zstrings
static uint8_t php_otic_flusher(uint8_t* ptr, size_t size)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    php_printf("Size: %lu\n", size);
    ZVAL_STRING(&sargs[0], (const char*)ptr);
    ZVAL_LONG(&sargs[1], size)
    sfci.param_count = 2;
    sfci.params = sargs;
    sfci.retval = &sret;
    if (zend_call_function(&sfci, &sfcc) != SUCCESS)
        return 0;
    return 1;
}

PHP_METHOD(OticPackChannel, __construct)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    oticPackChannel_object *intern = Z_OTICPACKCHANNELOBJ_P(id);
    if (intern)
        intern->oticPackChannel = emalloc(sizeof(otic_pack_channel_t));
}

PHP_METHOD(OticPackChannel, __destruct)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    oticPackChannel_object *intern = Z_OTICPACKCHANNELOBJ_P(id);
//    if (intern)
//        if (intern->oticPackChannel->base.state != OTIC_STATE_CLOSED)
//            otic_pack_channel_close(intern->oticPackChannel);
}

PHP_METHOD(OticPackChannel, inject)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    otic_str_t sensorName;
    otic_str_t sensorUnit;
    double timestamp = 0;
    zval* value;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dssz", &timestamp, &sensorName.ptr, &sensorName.size, &sensorUnit.ptr, &sensorUnit.size, &value) == FAILURE)
        return;
    oticPackChannel_object* intern = Z_OTICPACKCHANNELOBJ_P(id);
    if (!intern)
        return;
    php_printf("%f %s %s\n", timestamp, sensorName.ptr, sensorUnit.ptr);
    if (Z_TYPE_P(value) == IS_DOUBLE)
        RETURN_BOOL(otic_pack_channel_inject_d(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_DVAL_P(value)))
    else if (Z_TYPE_P(value) == IS_LONG)
        RETURN_BOOL(otic_pack_channel_inject_i(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_LVAL_P(value)))
    else if (Z_TYPE_P(value) == IS_STRING)
        RETURN_BOOL(otic_pack_channel_inject_s(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr, Z_STRVAL_P(value)))
    else if (Z_TYPE_P(value) == IS_NULL)
        RETURN_BOOL(otic_pack_channel_inject_n(intern->oticPackChannel, timestamp, sensorName.ptr, sensorUnit.ptr))
    else {
        zend_throw_error(zend_ce_exception, "Invalid Type", 0);
        return;
    }
}

PHP_METHOD(OticPackChannel, flush)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    oticPackChannel_object* intern;
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_FALSE
    if (!(intern = Z_OTICPACKCHANNELOBJ_P(id)))
        return;
    RETURN_TRUE
//    RETURN_BOOL(otic_pack_channel_flush(intern->oticPackChannel))
}

PHP_METHOD(OticPackChannel, close)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    oticPackChannel_object* intern;
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_FALSE
    if (!(intern = Z_OTICPACKCHANNELOBJ_P(id)))
        return;
    otic_pack_channel_close(intern->oticPackChannel);
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

ZEND_BEGIN_ARG_INFO_EX(FUNCTION_INJECT, 0, 0, 4)
    ZEND_ARG_INFO(0, timestamp)
    ZEND_ARG_INFO(0, sensorName)
    ZEND_ARG_INFO(0, sensorValue)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

const zend_function_entry oticPackChannel_methods[] = {
    PHP_ME(OticPackChannel, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(OticPackChannel, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    PHP_ME(OticPackChannel, inject, FUNCTION_INJECT, ZEND_ACC_PUBLIC)
    PHP_ME(OticPackChannel, flush, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(OticPackChannel, close, NULL, ZEND_ACC_PUBLIC)
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
    php_printf("%s\n", __PRETTY_FUNCTION__);
    oticPackChannel_object* my_obj = (oticPackChannel_object*)((char*)object - XtOffsetOf(oticPackChannel_object, std));
    zend_objects_destroy_object(object);
}

static void oticPackChannel_object_free(zend_object* object)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    oticPackChannel_object* my_obj = (oticPackChannel_object*)((char*)object - XtOffsetOf(oticPackChannel_object, std));
    // TODO: Check Reliabilty. Segfault? Sanitizer
    //efree(my_obj->oticPackChannel);
    zend_object_std_dtor(object);
}

PHP_METHOD(OticPack, __construct)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    oticPack_object* intern = Z_TSTOBJ_P(id);
    if (intern)
        intern->oticPack = emalloc(sizeof(otic_pack_t));
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &sfci, &sfcc) == FAILURE)
        RETURN_FALSE


    sfci.param_count = 2;
    sfci.params = sargs;
    sfci.retval = &sret;
    if (!otic_pack_init(intern->oticPack, php_otic_flusher))
        RETURN_FALSE
    RETURN_TRUE
}

PHP_METHOD(OticPack, __toString)
{
    zval* id = getThis();
    oticPack_object* intern;
    if (zend_parse_parameters_none() == FAILURE)
        RETURN_NULL()
    intern = Z_TSTOBJ_P(id);
    php_otic_flusher("Hallo Wrold", 10);
    if (intern)
        RETURN_STRING("Class: OticPack\n")
}

PHP_METHOD(OticPack, close)
{
    zval* id = getThis();
    oticPack_object* intern;
    if (zend_parse_parameters_none() == FAILURE)
    RETURN_NULL()
    intern = Z_TSTOBJ_P(id);
    if (intern){
        otic_pack_close(intern->oticPack);
    }
    php_printf("%s\n", __PRETTY_FUNCTION__);
    RETURN_NULL()
}

PHP_METHOD(OticPack, test)
{
    php_printf("%s\n", __PRETTY_FUNCTION__);
    zval* id = getThis();
    oticPack_object* intern;

    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;

    sfci = empty_fcall_info;
    sfcc = empty_fcall_info_cache;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &sfci, &sfcc) == FAILURE)
    RETURN_FALSE
    zval ret;
//    fci.param_count = 0;
//    fci.params = 0;
//    fci.retval = &ret;

    sfci.param_count = 0;
    sfci.params = 0;
    sfci.retval = &ret;

    if (SUCCESS != zend_call_function(&sfci, &sfcc))
    {

    } else {
    }

    php_printf("Got: %s\n", Z_STRVAL_P(&ret));
    RETURN_LONG(54)

//    if (SUCCESS != zend_call_function(&(fcallInfo), &fcallInfoCache))
//        RETURN_LONG(-2)
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
    if (intern)
        RETURN_BOOL(otic_pack_flush(intern->oticPack))
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
    intern = Z_TSTOBJ_P(id);
    if (!intern)
        return;
    php_printf("Channel ID: %ld, ChannelType: %s, ChannelFeatures: %ld\n", channelId, ZSTR_VAL(channelType), channelOptions);
    otic_pack_channel_t* oticPackChannel;
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

//    RETURN_ZVAL(id, 1u, 1)
//    zval ob;
    oticPackChannel_object* intern2 = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(oticPackChannel_ce));
    zend_object_std_init(&intern2->std, oticPackChannel_ce TSRMLS_CC);
    object_properties_init(&intern2->std, oticPackChannel_ce TSRMLS_CC);
    intern2->std.handlers = &oticPackChannel_handlers;
    intern2->oticPackChannel = otic_pack_defineChannel(intern->oticPack, type, channelId, 0x00);
    RETURN_OBJ(&intern2->std)

//    ZVAL_OBJ(&ob, &intern2->std);
//    RETURN_ZVAL(&ob, 1, 1)


    /*zend_object *oticPackChannelp = oticPackChannel_object_new(oticPackChannel_ce);
    ZVAL_OBJ(&ob, oticPackChannelp);
    RETURN_ZVAL(&ob, 1, 1)*/

    /*oticPackChannel_object* intern2 = (oticPackChannel_object*)ecalloc(1, sizeof(oticPackChannel_object) + zend_object_properties_size(oticPackChannel_ce));
    zend_object_std_init(&intern2->std, oticPackChannel_ce TSRMLS_CC);
    object_properties_init(&intern2->std, oticPackChannel_ce TSRMLS_CC);
    intern2->std.handlers = &oticPackChannel_handlers;
    intern2->oticPackChannel = otic_pack_defineChannel(intern->oticPack, type, channelId, channelOptions);
    RETURN_OBJ(&intern2->std)*/

//    zend_object* mi2 = oticPackChannel_object_new(oticPackChannel_ce);
//    RETURN_OBJ(mi2)


//    RETURN_OBJ(&mI->std)
//    RETURN_ZVAL(&mI, 1, 1);

//    RETURN_OBJ(&intern->std)

//    oticPackChannel = otic_pack_defineChannel(intern->oticPack, type, channelId, channelOptions);

//    oticPackChannel_object *object = malloc(sizeof(oticPackChannel_object));
//    object->oticPackChannel = otic_pack_defineChannel(intern->oticPack, type, channelId, channelOptions);

//    RETURN_ZVAL(object->std, 1, 1)
//    RETURN_ZVAL(id, 1u, 1)
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
        PHP_ME(OticPack, test, FUNCTION_PARAM, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, defineChannel, argInfo_defineChannel, ZEND_ACC_PUBLIC)
        PHP_ME(OticPack, closeChannel, argInfo_closeChannel, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

static FILE* oticFile;

static inline uint8_t flusher(uint8_t* content, size_t size)
{
    return fwrite(content, 1, size, oticFile) == 1;
}

PHP_METHOD(OticPackStream, __construct)
{
    zval* id = getThis();
    zend_string* fileName;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &fileName) == FAILURE) {
        RETURN_FALSE
    }
    oticPackStream_object *intern = Z_OTICSTREAM_OBJ_P(id);
    if (intern != NULL)
    {
        intern->content.oticPack = emalloc(sizeof(otic_pack_t));
        if (!(intern->content.file = fopen(ZSTR_VAL(fileName), "wb")))
            RETURN_FALSE
    }
    zend_string_release(fileName);
}

PHP_METHOD(OticPackStream, flush)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticPackStream_object* intern = Z_OTICSTREAM_OBJ_P(id);
    if (intern)
    {

    }
}

// TODO: Use linker function
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