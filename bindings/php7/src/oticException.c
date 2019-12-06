//
// Created by talpaadmin on 06.12.19.
//


#define OTIC_PHP_EXTNAME "otic"
#define OTIC_PHP_EXTVER  "1.0.0"

#include "oticException.h"
#include <zend_interfaces.h>
#include <zend_exceptions.h>
#include <ext/spl/spl_iterators.h>


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
    zend_update_property_str(libOticExceptions_ce, id, "message", strlen("message"), message);
    zend_update_property_long(libOticExceptions_ce, id, "code", strlen("code"), error);
    zend_string_release(message);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_void, 0, 0, 0)
ZEND_END_ARG_INFO()

const zend_function_entry libOticExceptions_functions [] = {
        PHP_ME(LibOticException, __construct, arginfo_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_FE_END
};