//
// Created by talpaadmin on 06.12.19.
//

#ifndef OTIC_PHP_OTIC_PHP_UNPACK_H
#define OTIC_PHP_OTIC_PHP_UNPACK_H

#include <php.h>
#include "core/unpack.h"

typedef struct oticUnpack_object_t
{
    otic_unpack_t* oticUnpack;
    zend_object std;
} oticUnpack_object;

extern zend_object_handlers oticUnpack_object_handlers;
extern zend_class_entry* oticUnpack_ce;
extern const zend_function_entry oticUnpack_methods[];

zend_object*        oticUnpack_object_new(zend_class_entry* ce TSRMLS_DC);
void                oticUnpack_object_destroy(zend_object* object);
void                oticUnpack_object_free(zend_object* object);


#endif //OTIC_PHP_OTIC_PHP_UNPACK_H
