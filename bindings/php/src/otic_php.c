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

// https://www.maplenerds.com/blog/2018/6/14/writing-php-72-extensions-with-c


zend_object_handlers otic_pack_handlers;
typedef struct
{
    otic_pack_t* oticPack;
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
        intern->oticPack = emalloc(sizeof(otic_pack_t));
    if (!otic_pack_init(intern->oticPack, flusher, metadata->val))
        php_printf("Failed");
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
    if (intern == NULL)
        return;
    if (Z_TYPE_P(value) == IS_DOUBLE)
        otic_pack_inject_d(intern->oticPack, timestamp, sensorName.content, sensorUnit.content, Z_DVAL_P(value));
    else if (Z_TYPE_P(value) == IS_LONG)
        otic_pack_inject_i(intern->oticPack, timestamp, sensorName.content, sensorUnit.content, Z_LVAL_P(value));
    else if (Z_TYPE_P(value) == IS_STRING)
        otic_pack_inject_s(intern->oticPack, timestamp, sensorName.content, sensorUnit.content, Z_STRVAL_P(value));
    else if (Z_TYPE_P(value) == IS_NULL)
        otic_pack_inject_n(intern->oticPack, timestamp, sensorName.content, sensorUnit.content);
    else {
        zend_throw_error(zend_ce_exception, "Invalid Type", 0);
        return;
    }
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
    otic_pack_inject_b(intern->oticPack, timestamp, _sensorName.content, _sensorUnit.content, (uint8_t*)_buffer.content, _buffer.size);
}

PHP_METHOD(OticPack, dump)
{
    if (zend_parse_parameters_none() == FAILURE){
        return;
    }
    zval* id = getThis();
    otic_pack_php_t* intern = Z_TSTOBJ_P(id);
    RETURN_BOOL(otic_pack_flush(intern->oticPack))
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


ZEND_GET_MODULE(otic)