//
// Created by talpaadmin on 06.12.19.
//

#include "otic_php_unpack.h"
#include "oticException.h"
#include <zend_exceptions.h>

// TODO:
#include <zend_generators.h>


zend_object_handlers oticUnpackChannel_object_handlers;



zend_object_handlers oticUnpack_object_handlers;

static inline oticUnpack_object* php_oticUnpack_obj_from_obj(zend_object* obj)
{
    return (oticUnpack_object*)((char*)(obj) - XtOffsetOf(oticUnpack_object, std));
}

#define T_OTICUNPACKOBJ_P(zv) php_oticUnpack_obj_from_obj(Z_OBJ_P((zv)))

zend_class_entry* oticUnpack_ce;
typedef struct oticUnpackChannel_object_t
{
    oticUnpackChannel_t oticUnpackChannel;
    zend_object std;
} oticUnpackChannel_object;

static inline oticUnpackChannel_object* php_oticUnpackChannel_obj_from_obj(zend_object* obj)
{
    return (oticUnpackChannel_object *) ((char *) (obj) - XtOffsetOf(oticUnpackChannel_object, std));
}

#define Z_OUNPACKCHAN(zv) php_oticUnpackChannel_obj_from_obj(Z_OBJ_P((zv)))

zend_class_entry* oticUnpackChannel_ce;

PHP_METHOD(OticUnpackChannel, __toString)
{
    RETURN_STRING(ZEND_NS_NAME("Otic", "OticUnpackChannel"))
}

PHP_METHOD(OticUnpackChannel, setOnCallback)
{
    zval* id =  getThis();
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &fci, &fcc) == FAILURE)
        return;
    oticUnpackChannel_object* intern = Z_OUNPACKCHAN(id);
    if (!intern)
        return;
}

PHP_METHOD(OticUnpackChannel, fetchSensor)
{

}


static inline uint8_t otic_php_unpack_fetcher(uint8_t* content, size_t size, void* data)
{
    php_stream_read((php_stream*)data, (char*)content, size);
    return 1;
}

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

PHP_METHOD(OticUnpack, selectChannel)
{

}

PHP_METHOD(OticUnpack, unselectChannel)
{

}

PHP_METHOD(OticUnpack, parse)
{
    zval* id = getThis();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    oticUnpack_object* intern = T_OTICUNPACKOBJ_P(id);
    if (!intern)
        return;
    if (!otic_unpack_parse(intern->oticUnpack))
        zend_throw_exception(libOticExceptions_ce, "", intern->oticUnpack->error);
}

const zend_function_entry oticUnpack_methods[] = {
        PHP_ME(OticUnpack, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(OticUnpack, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(OticUnpack, selectChannel, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(OticUnpack, unselectChannel, NULL, ZEND_ACC_PUBLIC)
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
    efree(myObj->oticUnpack);
    zend_object_std_dtor(object);
}