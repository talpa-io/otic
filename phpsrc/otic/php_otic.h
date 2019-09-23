/* otic extension for PHP */

#include "otic.h"

#ifndef PHP_OTIC_H
# define PHP_OTIC_H

extern zend_module_entry otic_module_entry;
# define phpext_otic_ptr &otic_module_entry

# define PHP_OTIC_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_OTIC)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

// reader

typedef struct _otic_reader_object {
    otic_reader r;
    otic_result res;
    zend_object zo;
} otic_reader_object;

static inline otic_reader_object* php_otic_reader_fetch_object(zend_object *obj) {
	return (otic_reader_object *)((char*)(obj) - XtOffsetOf(otic_reader_object, zo));
}


// writer
//
typedef struct _otic_writer_object {
    otic_writer w;
    zend_object zo;
} otic_writer_object;

static inline otic_writer_object* php_otic_writer_fetch_object(zend_object *obj) {
	return (otic_writer_object *)((char*)(obj) - XtOffsetOf(otic_writer_object, zo));
}

#endif	/* PHP_OTIC_H */
