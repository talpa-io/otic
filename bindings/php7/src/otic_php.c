/**
 * Useful Links:
 *
 * About Zend Extensions in C with the Zend Engine
 * http://www.phpinternalsbook.com/php7/extensions_design/zend_extensions.html
 *
 * About Zend functions in C
 *
 * The following link contains an error. While registring function `FE_END` needs to be added at the end
 * of the array, something that wasn't shown in the tutorial
 * http://www.phpinternalsbook.com/php7/extensions_design/php_functions.html#registering-php-functions
 */

#include "otic_php.h"
#include "otic_exception.h"
#include "otic_php_pack.h"
#include "otic_php_unpack.h"
#include <zend_exceptions.h>
#include <zend_string.h>
#include <zend_generators.h>

#define OTIC_PHP_EXTNAME "otic"
#define OTIC_PHP_EXTVER  "1.0.0"
#define OTIC_NS_NAME "Otic"


PHP_FUNCTION(getLibOticVersion)
{
    if (zend_parse_parameters_none() == FAILURE)
        return;
    char buffer[12] = {};
    sprintf(buffer, "%u.%u.%u", OTIC_VERSION_MAJOR, OTIC_VERSION_MINOR, OTIC_VERSION_PATCH);
    RETURN_STRING(buffer)
}

static const zend_function_entry oticFunctions[] =
{
    PHP_FE(getLibOticVersion, NULL)
    PHP_FE_END
};

#define STRING_AND_LENGTH(str) #str, strlen(#str)
#define TO_OTIC_ERROR(str) STRING_AND_LENGTH(str), OTIC_ERROR_##str
#define OTIC_PHP_EXCEPTION(str) zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(str))

PHP_MINIT_FUNCTION(otic)
{
    zend_class_entry temp_ce;
    INIT_NS_CLASS_ENTRY(temp_ce, OTIC_NS_NAME, "LibOticException", libOticExceptions_functions);
    libOticExceptions_ce = zend_register_internal_class(&temp_ce TSRMLS_CC);
    libOticExceptions_ce = zend_register_internal_class_ex(&temp_ce, zend_exception_get_default(TSRMLS_C));
    OTIC_PHP_EXCEPTION(NONE);
    OTIC_PHP_EXCEPTION(INVALID_POINTER);
    OTIC_PHP_EXCEPTION(BUFFER_OVERFLOW);
    OTIC_PHP_EXCEPTION(INVALID_TIMESTAMP);
    OTIC_PHP_EXCEPTION(ENTRY_INSERTION_FAILURE);
    OTIC_PHP_EXCEPTION(ZSTD);
    OTIC_PHP_EXCEPTION(FLUSH_FAILED);
    OTIC_PHP_EXCEPTION(INVALID_FILE);
    OTIC_PHP_EXCEPTION(DATA_CORRUPTED);
    OTIC_PHP_EXCEPTION(VERSION_UNSUPPORTED);
    OTIC_PHP_EXCEPTION(ROW_COUNT_MISMATCH);
    OTIC_PHP_EXCEPTION(INVALID_ARGUMENT);
    OTIC_PHP_EXCEPTION(AT_INVALID_STATE);
    zend_declare_class_constant_long(libOticExceptions_ce, "EOF", 3, OTIC_ERROR_EOF); // EOF already defined als Macro
    OTIC_PHP_EXCEPTION(ALLOCATION_FAILURE);

    zend_class_entry temp2_ce;
    INIT_NS_CLASS_ENTRY(temp2_ce, OTIC_NS_NAME, "OticException", NULL)
    oticExceptions_ce = zend_register_internal_class_ex(&temp2_ce, zend_exception_get_default(TSRMLS_C));

    zend_class_entry temp3_ce;
    INIT_NS_CLASS_ENTRY(temp3_ce, OTIC_NS_NAME, "OticPack", oticPack_methods);
    oticPack_ce = zend_register_internal_class(&temp3_ce TSRMLS_CC);
    oticPack_ce->create_object = oticPack_object_new;
    memcpy(&oticPack_object_handlers, zend_get_std_object_handlers(), sizeof(oticPack_object_handlers));
    oticPack_object_handlers.free_obj = oticPack_object_free;
    oticPack_object_handlers.dtor_obj = oticPack_object_destroy;
    oticPack_object_handlers.offset = XtOffsetOf(oticPack_object, std);

    zend_class_entry temp4_ce;
    INIT_NS_CLASS_ENTRY(temp4_ce, OTIC_NS_NAME, "OticPackChannel", oticPackChannel_methods)
    oticPackChannel_ce = zend_register_internal_class(&temp4_ce TSRMLS_CC);
    oticPackChannel_ce->create_object = oticPackChannel_object_new;
    memcpy(&oticPackChannel_object_handlers, zend_get_std_object_handlers(), sizeof(oticPackChannel_object_handlers));
    oticPackChannel_object_handlers.free_obj = oticPackChannel_object_free;
    oticPackChannel_object_handlers.dtor_obj = oticPackChannel_object_destroy;
    oticPackChannel_object_handlers.offset   = XtOffsetOf(oticPackChannel_object, std);

    zend_declare_class_constant_long(oticPackChannel_ce, STRING_AND_LENGTH("TYPE_SENSOR"), OTIC_CHANNEL_TYPE_SENSOR);
    zend_declare_class_constant_long(oticPackChannel_ce, STRING_AND_LENGTH("TYPE_BINARY"), OTIC_CHANNEL_TYPE_BINARY);

    zend_class_entry temp5_ce;
    INIT_NS_CLASS_ENTRY(temp5_ce, OTIC_NS_NAME, "OticUnpack", oticUnpack_methods)
    oticUnpack_ce = zend_register_internal_class(&temp5_ce TSRMLS_CC);
    oticUnpack_ce->create_object = oticUnpack_object_new;
    memcpy(&oticUnpack_object_handlers, zend_get_std_object_handlers(), sizeof(oticUnpack_object_handlers));
    oticUnpack_object_handlers.free_obj = oticUnpack_object_free;
    oticUnpack_object_handlers.dtor_obj = oticUnpack_object_destroy;
    oticUnpack_object_handlers.offset   = XtOffsetOf(oticUnpack_object, std);

    zend_class_entry temp6_ce;
    INIT_NS_CLASS_ENTRY(temp6_ce, OTIC_NS_NAME, "OticUnpackChannel", oticUnpackChannel_methods)
    oticUnpackChannel_ce = zend_register_internal_class(&temp6_ce TSRMLS_CC);
    oticUnpackChannel_ce->create_object = oticUnpackChannel_object_new;
    memcpy(&oticUnpackChannel_object_handlers, zend_get_std_object_handlers(), sizeof(oticUnpackChannel_object_handlers));
    oticUnpackChannel_object_handlers.free_obj = oticUnpackChannel_object_free;
    oticUnpackChannel_object_handlers.dtor_obj = oticUnpackChannel_object_destroy;
    oticUnpackChannel_object_handlers.offset = XtOffsetOf(oticUnpackChannel_object, std);

    return SUCCESS;
}

zend_module_entry otic_module_entry =
        {
#if ZEND_MODULE_API_NO >= 20010901
                STANDARD_MODULE_HEADER,
#endif
                OTIC_PHP_EXTNAME,
                oticFunctions,
                PHP_MINIT(otic),
                NULL,
                NULL,
                NULL,
                NULL,
#if ZEND_MODULE_API_NO >= 20010901
                OTIC_PHP_EXTVER,
#endif
                STANDARD_MODULE_PROPERTIES
        };

ZEND_GET_MODULE(otic)

#undef OTIC_PHP_EXTNAME
#undef OTIC_PHP_EXTVER
#undef OTIC_NS_NAME

#undef STRING_AND_LENGTH
#undef TO_OTIC_ERROR
#undef OTIC_PHP_EXCEPTION
