#ifndef OTIC_PHP_PORTABILITY_H
#define OTIC_PHP_PORTABILITY_H

#ifdef OTIC_PHP_WIN32
#define PHP_OTIC_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_OTIC_API __attribute__((visibility("default")))
#else
#define PHP_OTIC_API
#endif

#endif //OTIC_PHP_PORTABILITY_H
