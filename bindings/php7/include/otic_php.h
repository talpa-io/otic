//
// Created by talpaadmin on 06.12.19.
//

#ifndef OTIC_OTIC_PHP_H
#define OTIC_OTIC_PHP_H

#include <php.h>
extern zend_module_entry otic_module_entry;
#define otic_module_ptr &otic_module_entry;
#define php_ext_otic_ptr otic_module_ptr;

#endif //OTIC_OTIC_PHP_H
