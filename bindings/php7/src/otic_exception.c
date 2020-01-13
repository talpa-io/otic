#include "otic_exception.h"
#include <zend_interfaces.h>
#include <zend_exceptions.h>
#include <string.h>

#define STRING_AND_LENGTH(str) #str,strlen(#str)
#define TO_OTIC_ERROR(str) STRING_AND_LENGTH(str), OTIC_ERROR_##str

/**
 * Useful links:
 *
 * Basic structure of classes
 * http://www.phpinternalsbook.com/php5/classes_objects/simple_classes.html
 *
 * zend_string
 * http://www.phpinternalsbook.com/php7/internal_types/strings/zend_strings.html
 */

zend_class_entry* oticExceptions_ce;
zend_class_entry* libOticExceptions_ce;

PHP_METHOD(LibOticException, __construct)
{
    zval* id = getThis();
    long error;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &error) == FAILURE)
        RETURN_FALSE
    zend_string* message = zend_string_init(otic_strError(error), strlen(otic_strError(error)), 0);
    zend_update_property_str(libOticExceptions_ce, id, STRING_AND_LENGTH(message), message);
    zend_update_property_long(libOticExceptions_ce, id,STRING_AND_LENGTH(code), error);
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(NONE));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(INVALID_POINTER));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(BUFFER_OVERFLOW));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(INVALID_TIMESTAMP));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(ENTRY_INSERTION_FAILURE));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(ZSTD));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(FLUSH_FAILED));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(EOF));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(INVALID_FILE));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(DATA_CORRUPTED));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(VERSION_UNSUPPORTED));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(ROW_COUNT_MISMATCH));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(INVALID_ARGUMENT));
    zend_declare_class_constant_long(libOticExceptions_ce, TO_OTIC_ERROR(AT_INVALID_STATE));
    zend_string_release(message);
}

ZEND_BEGIN_ARG_INFO_EX(ARGINFO_LONG, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, errNo, IS_LONG, 0)
ZEND_END_ARG_INFO()

const zend_function_entry libOticExceptions_functions [] = {
        PHP_ME(LibOticException, __construct, ARGINFO_LONG, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_FE_END
};

#undef STRING_AND_LENGTH
#undef TO_OTIC_ERROR