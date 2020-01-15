//
// Created by talpaadmin on 06.12.19.
//

#ifndef OTIC_OTIC_EXCEPTION_H
#define OTIC_OTIC_EXCEPTION_H

#include <php.h>
#include "utility/errHand.h"

extern const zend_function_entry libOticExceptions_functions[];
extern zend_class_entry* oticExceptions_ce;
extern zend_class_entry *libOticExceptions_ce;

#define otic_php_throw_libOticException(errorCode) zend_throw_exception(libOticExceptions_ce, otic_strError(errorCode), errorCode)
#define otic_php_throw_oticException(errorMessage, errorCode) zend_throw_exception(oticExceptions_ce, errorMessage, errorCode)


#endif //OTIC_OTIC_EXCEPTION_H
