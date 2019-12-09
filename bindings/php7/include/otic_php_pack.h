//
// Created by talpaadmin on 06.12.19.
//

#ifndef OTIC_PHP_OTIC_PHP_PACK_H
#define OTIC_PHP_OTIC_PHP_PACK_H

#include <php.h>
#include "core/pack.h"


typedef struct oticPackChannel_object_t
{
    otic_pack_channel_t* oticPackChannel;
    zend_object std;
} oticPackChannel_object;

extern zend_object_handlers oticPackChannel_object_handlers;
extern zend_class_entry* oticPackChannel_ce;
extern const zend_function_entry oticPackChannel_methods[];

zend_object*    oticPackChannel_object_new(zend_class_entry* ce TSRMLS_DC);
void            oticPackChannel_object_destroy(zend_object* object);
void            oticPackChannel_object_free(zend_object* object);


typedef struct oticPack_object_t
{
    otic_pack_t* oticPack;
    zend_object std;
} oticPack_object;

extern zend_class_entry* oticPack_ce;
extern zend_object_handlers oticPack_object_handlers;
extern const    zend_function_entry oticPack_methods[];

zend_object*    oticPack_object_new(zend_class_entry* ce TSRMLS_DC);
void            oticPack_object_destroy(zend_object* object);
void            oticPack_object_free(zend_object* object);

#endif //OTIC_PHP_OTIC_PHP_PACK_H
