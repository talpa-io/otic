//
// Created by talpaadmin on 06.12.19.
//

#include "otic_php.h"
#include "oticException.h"
#include "otic_php_pack.h"
#include "otic_php_unpack.h"
#include <zend_exceptions.h>

#define OTIC_PHP_EXTNAME "otic"
#define OTIC_PHP_EXTVER  "1.0.0"


PHP_MINIT_FUNCTION(otic)
{
    zend_class_entry temp_ce;
    INIT_CLASS_ENTRY(temp_ce, "LibOticException", libOticExceptions_functions)
    libOticExceptions_ce = zend_register_internal_class(&temp_ce TSRMLS_CC);
    libOticExceptions_ce = zend_register_internal_class_ex(&temp_ce, zend_exception_get_default(TSRMLS_C));

    zend_class_entry temp2_ce;
    INIT_CLASS_ENTRY(temp2_ce, "OticException", NULL)
    oticExceptions_ce = zend_register_internal_class_ex(&temp2_ce, zend_exception_get_default(TSRMLS_C));

    zend_class_entry temp3_ce;
    INIT_CLASS_ENTRY(temp3_ce, "OticPack", oticPack_methods)
    oticPack_ce = zend_register_internal_class(&temp3_ce TSRMLS_CC);
    oticPack_ce->create_object = oticPack_object_new;
    memcpy(&oticPack_object_handlers, zend_get_std_object_handlers(), sizeof(oticPack_object_handlers));
    oticPack_object_handlers.free_obj = oticPack_object_free;
    oticPack_object_handlers.dtor_obj = oticPack_object_destroy;
    oticPack_object_handlers.offset = XtOffsetOf(oticPack_object, std);

    zend_class_entry temp4_ce;
    INIT_CLASS_ENTRY(temp4_ce, "OticPackChannel", oticPackChannel_methods)
    oticPackChannel_ce = zend_register_internal_class(&temp4_ce TSRMLS_CC);
    oticPackChannel_ce->create_object = oticPackChannel_object_new;
    memcpy(&oticPackChannel_object_handlers, zend_get_std_object_handlers(), sizeof(oticPackChannel_object_handlers));
    oticPackChannel_object_handlers.free_obj = oticPackChannel_object_free;
    oticPackChannel_object_handlers.dtor_obj = oticPackChannel_object_destroy;
    oticPackChannel_object_handlers.offset   = XtOffsetOf(oticPackChannel_object, std);

    zend_class_entry temp5_ce;
    INIT_CLASS_ENTRY(temp5_ce, "OticUnpack", oticUnpack_methods)
    oticUnpack_ce = zend_register_internal_class(&temp5_ce TSRMLS_CC);
    oticUnpack_ce->create_object = oticUnpack_object_new;
    memcpy(&oticUnpack_object_handlers, zend_get_std_object_handlers(), sizeof(oticUnpack_object_handlers));
    oticUnpack_object_handlers.free_obj = oticUnpack_object_free;
    oticUnpack_object_handlers.dtor_obj = oticUnpack_object_destroy;
    oticUnpack_object_handlers.offset   = XtOffsetOf(oticUnpack_object, std);


    return SUCCESS;
}

zend_module_entry otic_module_entry =
        {
#if ZEND_MODULE_API_NO >= 20010901
                STANDARD_MODULE_HEADER,
#endif
                OTIC_PHP_EXTNAME,
                NULL,
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