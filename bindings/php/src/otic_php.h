#ifndef PHP_OTIC_H
#define PHP_OTIC_H 1

#define PHP_OTIC_WORLD_VERSION "1.0"
#define PHP_OTIC_WORLD_EXTNAME "otic"
#include <php.h>

extern zend_module_entry otic_world_entry;
#define otic_module_ptr &otic_module_entry
#define phpext_otic_ptr otic_module_ptr

#endif