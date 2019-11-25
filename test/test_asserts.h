//
// Created by hp on 9/19/19.
//

#ifndef OTIC_TEST_ASSERTS_H
#define OTIC_TEST_ASSERTS_H

#include <stdio.h>
#include <stdlib.h>


#define OTIC_ASSERT_THROW( condition )                             \
{                                                                   \
  if( !( condition ) )                                              \
  {                                                                 \
    fprintf(stderr, "ASSERTION FAILED! %s: %i in %s.", __FILE__, __LINE__, __PRETTY_FUNCTION__);\
    exit(1);                                                         \
  }                                                                 \
}

#define OTIC_ASSERT_EQUAL( x, y )                                  \
{                                                                   \
    if( ( x ) != ( y ) ) {                                          \
        fprintf(stderr, "%s: %i in %s: %s != %s.", __FILE__, __LINE__, __PRETTY_FUNCTION__, (#x), (#y));\
        exit(1);                                                     \
    }                                                               \
}                                                                   \


#endif //OTIC_TEST_ASSERTS_H
