/* otic extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "ext/standard/info.h"
#include "php_otic.h"

#include "otic_all.h"



PHP_RINIT_FUNCTION(otic)
{
#if defined(ZTS) && defined(COMPILE_DL_OTIC)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    return SUCCESS;
}

// CLASS OBJECTS


static zend_class_entry *otic_reader_object_ce;
static zend_class_entry *otic_writer_object_ce;

// READER

#define Z_OTIC_READER_P(zv) php_otic_reader_fetch_object(Z_OBJ_P((zv)))

static zend_object_handlers otic_reader_object_handlers;

static zend_object *php_otic_reader_object_new(zend_class_entry *class_type)
{
    otic_reader_object *intern;

    intern = ecalloc(1, sizeof(otic_reader_object) + zend_object_properties_size(class_type));
    zend_object_std_init(&intern->zo, class_type);
    object_properties_init(&intern->zo, class_type);
    intern->zo.handlers = &otic_reader_object_handlers;

    return &intern->zo;
}

static void php_otic_reader_object_free_storage(zend_object *object) {
    otic_reader_object *intern = php_otic_reader_fetch_object(object);
    if (!intern) {
        return;
    }
    zend_object_std_dtor(&intern->zo);
    if (intern->r) {
        otic_reader_close(intern->r);
    }
}

PHP_METHOD(OticReaderRaw, open)
{
    char *resolved_path;
    zend_string *filename;
    zval *self = getThis();
    otic_reader_object *ze_obj = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "P", &filename) == FAILURE) {
        return;
    }

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }
    ze_obj = Z_OTIC_READER_P(self);

    if (ZSTR_LEN(filename) == 0) {
        zend_throw_exception(zend_ce_exception, "Empty string as source", 0);
        return;
    }

    if (php_check_open_basedir(ZSTR_VAL(filename))) {
        RETURN_FALSE;
    }

    if (!(resolved_path = expand_filepath(ZSTR_VAL(filename), NULL))) {
        RETURN_FALSE;
    }

    if (ze_obj->r) {
        /* we already have an opened file */
        zend_throw_exception(zend_ce_exception, "reader already open", 0);
        return;
    }
    otic_reader r = otic_reader_open_filename(resolved_path);
    if (!r) {
        zend_throw_exception(zend_ce_exception, "error opening file!", 0);
    }
    ze_obj->r = r;
    RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_reader_open, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

size_t _read_callback(void* userdata, char *data, size_t size) {
    return php_stream_read((php_stream*)userdata, data, size);
}

PHP_METHOD(OticReaderRaw, open_stream)
{
    zval *zstream;
    php_stream *stream;

    zval *self = getThis();
    otic_reader_object *ze_obj = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zstream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, zstream);

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }
    ze_obj = Z_OTIC_READER_P(self);

    if (ze_obj->r) {
        /* we already have an opened file */
        zend_throw_exception(zend_ce_exception, "reader already open", 0);
        return;
    }

    otic_reader r = otic_reader_open(_read_callback, (void*)stream);
    if (!r) {
        zend_throw_exception(zend_ce_exception, "error opening file!", 0);
    }
    ze_obj->r = r;
    RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_reader_open_stream, 0, 0, 1)
    ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO()

PHP_METHOD(OticReaderRaw, close)
{
    zval *self = getThis();
    otic_reader_object *ze_obj = NULL;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }

    ze_obj = Z_OTIC_READER_P(self);
    if (ze_obj->r) {
        int err = otic_reader_close(ze_obj->r);
        ze_obj->r = NULL;
        if (!err) {
            RETURN_TRUE;
        }
    }

    zend_throw_exception(zend_ce_exception, "error closing reader", 0);
    return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_reader_close, 0, 0, 0)
ZEND_END_ARG_INFO()


PHP_METHOD(OticReaderRaw, read)
{
    zval *self = getThis();
    otic_reader_object *ze_obj = NULL;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }

    ze_obj = Z_OTIC_READER_P(self);
    if (!ze_obj->r) {
        zend_throw_exception(zend_ce_exception, "reader not open!", 0);
        return;
    }
    otic_result res = otic_reader_next(ze_obj->r);
    ze_obj->res = res;
    if (!res) {
        if (otic_reader_geterror(ze_obj->r)) {
            zend_throw_exception(zend_ce_exception, "error when reading next entry!", 0);
            return;
        }
        RETURN_FALSE;
    }
    array_init(return_value);
    // colname
    char* colname = NULL;
    size_t colname_size = otic_result_get_colname(res, &colname);
    add_assoc_stringl(return_value, "colname", colname, colname_size);

    // metadata
    char* metadata = NULL;
    size_t metadata_size = otic_result_get_metadata(res, &metadata);
    if (metadata) {
        add_assoc_stringl(return_value, "metadata", metadata, metadata_size);
    }

    // time
    double ts = (double)otic_result_get_epoch(res);
    ts += 1e-9 * (double)otic_result_get_nanoseconds(res);
    add_assoc_double(return_value, "ts", ts);

    // value
    int typ = otic_result_get_type(res);
    if (typ == OTIC_TYPE_INT) {
        add_assoc_long(return_value, "value", otic_result_get_long(res));
    } else if (typ == OTIC_TYPE_DOUBLE) {
        add_assoc_double(return_value, "value", otic_result_get_double(res));
    } else if (typ == OTIC_TYPE_NULL) {
        add_assoc_null(return_value, "value");
    } else {
        char* name = NULL;
        size_t size = otic_result_get_string(res, &name);
        add_assoc_stringl(return_value, "value", name, size);
    }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_reader_read, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(OticReaderRaw, ignore_previous_column) {
    zval *self = getThis();
    otic_reader_object *ze_obj = NULL;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }

    ze_obj = Z_OTIC_READER_P(self);
    if (!ze_obj->res) {
        zend_throw_exception(zend_ce_exception, "no previous column!", 0);
        return;
    }

    otic_result_ignore_column_from_now_on(ze_obj->res);
    RETURN_TRUE;
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_reader_ignore_previous_column, 0, 0, 0)
ZEND_END_ARG_INFO()


PHP_METHOD(OticReaderRaw, get_closing_timestamp)
{
    zval *self = getThis();
    otic_reader_object *ze_obj = NULL;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }

    ze_obj = Z_OTIC_READER_P(self);
    if (!ze_obj->r) {
        zend_throw_exception(zend_ce_exception, "reader not open!", 0);
        return;
    }

    // time
    double ts = (double)otic_reader_closing_epoch(ze_obj->r);
    ts += 1e-9 * (double)otic_reader_closing_nanoseconds(ze_obj->r);
    if (ts == 0) {
        zend_throw_exception(zend_ce_exception, "reader not at end!", 0);
        return;
    }

    RETURN_DOUBLE(ts);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_reader_get_closing_timestamp, 0, 0, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry otic_reader_class_functions[] = {
    PHP_ME(OticReaderRaw, open, arginfo_otic_reader_open, ZEND_ACC_PUBLIC)
    PHP_ME(OticReaderRaw, open_stream, arginfo_otic_reader_open_stream, ZEND_ACC_PUBLIC)
    PHP_ME(OticReaderRaw, close, arginfo_otic_reader_close, ZEND_ACC_PUBLIC)
    PHP_ME(OticReaderRaw, read, arginfo_otic_reader_read, ZEND_ACC_PUBLIC)
    PHP_ME(OticReaderRaw, ignore_previous_column, arginfo_otic_reader_ignore_previous_column, ZEND_ACC_PUBLIC)
    PHP_ME(OticReaderRaw, get_closing_timestamp, arginfo_otic_reader_get_closing_timestamp, ZEND_ACC_PUBLIC)
    PHP_FE_END
};



// WRITER

#define Z_OTIC_WRITER_P(zv) php_otic_writer_fetch_object(Z_OBJ_P((zv)))

static zend_object_handlers otic_writer_object_handlers;

static zend_object *php_otic_writer_object_new(zend_class_entry *class_type)
{
    otic_writer_object *intern;

    intern = ecalloc(1, sizeof(otic_writer_object) + zend_object_properties_size(class_type));
    zend_object_std_init(&intern->zo, class_type);
    object_properties_init(&intern->zo, class_type);
    intern->zo.handlers = &otic_writer_object_handlers;

    return &intern->zo;
}

static void php_otic_writer_object_free_storage(zend_object *object) {
    otic_writer_object *intern = php_otic_writer_fetch_object(object);
    if (!intern) {
        return;
    }
    zend_object_std_dtor(&intern->zo);
    if (intern->w) {
        otic_writer_close(intern->w);
    }
}

PHP_METHOD(OticWriterRaw, open)
{
    char *resolved_path;
    zend_string *filename;
    zval *self = getThis();
    otic_writer_object *ze_obj = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "P", &filename) == FAILURE) {
        return;
    }

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }
    ze_obj = Z_OTIC_WRITER_P(self);

    if (ZSTR_LEN(filename) == 0) {
        zend_throw_exception(zend_ce_exception, "Empty string as source", 0);
        return;
    }

    if (php_check_open_basedir(ZSTR_VAL(filename))) {
        RETURN_FALSE;
    }

    if (!(resolved_path = expand_filepath(ZSTR_VAL(filename), NULL))) {
        RETURN_FALSE;
    }

    if (ze_obj->w) {
        /* we already have an opened file */
        zend_throw_exception(zend_ce_exception, "writer already open", 0);
        return;
    }
    otic_writer w = otic_writer_open_filename(resolved_path);
    if (!w) {
        zend_throw_exception(zend_ce_exception, "error opening file!", 0);
    }
    ze_obj->w = w;
    RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_writer_open, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

PHP_METHOD(OticWriterRaw, close)
{
    zval *self = getThis();
    otic_writer_object *ze_obj = NULL;

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }

    ze_obj = Z_OTIC_WRITER_P(self);
    if (ze_obj->w) {
        int err = otic_writer_close(ze_obj->w);
        ze_obj->w = NULL;
        if (!err) {
            RETURN_TRUE;
        }
    }

    zend_throw_exception(zend_ce_exception, "error closing writer", 0);
    return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_writer_close, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(OticWriterRaw, define_column)
{
    char *colname;
    size_t colnamelen;

    char *metadata = NULL;
    size_t metadatalen;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(colname, colnamelen);
        Z_PARAM_OPTIONAL;
        Z_PARAM_STRING(metadata, metadatalen);
    ZEND_PARSE_PARAMETERS_END();

    zval *self = getThis();
    otic_writer_object *ze_obj = NULL;

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }
    ze_obj = Z_OTIC_WRITER_P(self);

    if (!ze_obj->w) {
        zend_throw_exception(zend_ce_exception, "writer not open!", 0);
        return;
    }

    otic_column col = NULL;
    int error;
    if (!metadata) {
        error = otic_register_column(ze_obj->w, colname, &col);
    } else {
        error = otic_register_column_metadata(ze_obj->w, colname, metadata, metadatalen, &col);
    }
    if (error) {
        zend_throw_exception(zend_ce_exception, "couldn't register column", 0);
        return;
    }

    RETURN_LONG(otic_column_get_index(col));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_writer_define_column, 0, 0, 1)
    ZEND_ARG_INFO(0, colname)
    ZEND_ARG_INFO(0, metadat)
ZEND_END_ARG_INFO()


PHP_METHOD(OticWriterRaw, write)
{
    long colindex = 0;
    double ts = 0.0;

    zval *value;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_LONG(colindex);
        Z_PARAM_DOUBLE(ts);
        Z_PARAM_ZVAL(value);
    ZEND_PARSE_PARAMETERS_END();

    zval *self = getThis();
    otic_writer_object *ze_obj = NULL;

    if (!self) {
        zend_throw_exception(zend_ce_exception, "no this!", 0);
        return;
    }
    ze_obj = Z_OTIC_WRITER_P(self);

    if (!ze_obj->w) {
        zend_throw_exception(zend_ce_exception, "writer not open!", 0);
        return;
    }

    time_t t = (time_t)ts;
    long ns = (long)((ts - (double)t) * 1e9);
    int error = 0;
    if (Z_TYPE_P(value) == IS_LONG) {
        error = otic_write_long_index(ze_obj->w, colindex, t, ns, Z_LVAL_P(value));
    } else if (Z_TYPE_P(value) == IS_DOUBLE) {
        error = otic_write_double_index(ze_obj->w, colindex, t, ns, Z_DVAL_P(value));
    } else if (Z_TYPE_P(value) == IS_STRING) {
        error = otic_write_string_index(ze_obj->w, colindex, t, ns, Z_STRVAL_P(value), Z_STRLEN_P(value));
    } else if (Z_TYPE_P(value) == IS_NULL) {
        error = otic_write_null_index(ze_obj->w, colindex, t, ns);
    } else {
        zend_throw_exception(zend_ce_exception, "type not supported", 0);
        return;
    }
    if (error == OTIC_ERROR_TIMESTAMP_DECREASED) {
        zend_throw_exception(zend_ce_exception, "timestamp decreased!", 0);
        return;
    } else if (error) {
        zend_throw_exception(zend_ce_exception, "unknown exception occurred when writing", 0);
        return;
    }
    RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_otic_writer_write, 0, 0, 1)
    ZEND_ARG_INFO(0, colindex)
    ZEND_ARG_INFO(0, time)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static const zend_function_entry otic_writer_class_functions[] = {
    PHP_ME(OticWriterRaw, open, arginfo_otic_writer_open, ZEND_ACC_PUBLIC)
    PHP_ME(OticWriterRaw, define_column, arginfo_otic_writer_define_column, ZEND_ACC_PUBLIC)
    PHP_ME(OticWriterRaw, write, arginfo_otic_writer_write, ZEND_ACC_PUBLIC)
    PHP_ME(OticWriterRaw, close, arginfo_otic_writer_close, ZEND_ACC_PUBLIC)
    PHP_FE_END
};



// INIT

PHP_MINIT_FUNCTION(otic) {
    zend_class_entry ce_reader;

    memcpy(&otic_reader_object_handlers, &std_object_handlers, sizeof(zend_object_handlers));
    otic_reader_object_handlers.offset = XtOffsetOf(otic_reader_object, zo);
    otic_reader_object_handlers.free_obj = php_otic_reader_object_free_storage;
    otic_reader_object_handlers.clone_obj = NULL;
    //otic_reader_object_handlers.get_property_ptr_ptr = php_otic_reader_get_property_ptr_ptr;

    //otic_reader_object_handlers.get_gc          = php_otic_reader_get_gc;
    //otic_reader_object_handlers.get_properties = php_otic_reader_get_properties;
    //otic_reader_object_handlers.read_property  = php_otic_reader_read_property;
    //otic_reader_object_handlers.has_property   = php_otic_reader_has_property;

    INIT_CLASS_ENTRY(ce_reader, "OticReaderRaw", otic_reader_class_functions);
    ce_reader.create_object = php_otic_reader_object_new;
    otic_reader_object_ce = zend_register_internal_class(&ce_reader);


    zend_class_entry ce_writer;
    memcpy(&otic_writer_object_handlers, &std_object_handlers, sizeof(zend_object_handlers));
    otic_writer_object_handlers.offset = XtOffsetOf(otic_writer_object, zo);
    otic_writer_object_handlers.free_obj = php_otic_writer_object_free_storage;
    otic_writer_object_handlers.clone_obj = NULL;
    //otic_writer_object_handlers.get_property_ptr_ptr = php_otic_writer_get_property_ptr_ptr;

    //otic_writer_object_handlers.get_gc          = php_otic_writer_get_gc;
    //otic_writer_object_handlers.get_properties = php_otic_writer_get_properties;
    //otic_writer_object_handlers.read_property  = php_otic_writer_read_property;
    //otic_writer_object_handlers.has_property   = php_otic_writer_has_property;

    INIT_CLASS_ENTRY(ce_writer, "OticWriterRaw", otic_writer_class_functions);
    ce_writer.create_object = php_otic_writer_object_new;
    otic_writer_object_ce = zend_register_internal_class(&ce_writer);

    return SUCCESS;
}

PHP_MINFO_FUNCTION(otic)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "otic support", "enabled");
    php_info_print_table_end();
}


static const zend_function_entry otic_functions[] = {
    PHP_FE_END
};

zend_module_entry otic_module_entry = {
    STANDARD_MODULE_HEADER,
    "otic", /* Extension name */
    otic_functions,   /* zend_function_entry */
    PHP_MINIT(otic),   /* PHP_MINIT - Module initialization */
    NULL,       /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(otic),   /* PHP_RINIT - Request initialization */
    NULL,       /* PHP_RSHUTDOWN - Request shutdown */
    PHP_MINFO(otic),   /* PHP_MINFO - Module info */
    PHP_OTIC_VERSION,  /* Version */
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_OTIC
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(otic)
#endif
