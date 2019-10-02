//
// Created by talpaadmin on 02.10.19.
//

#ifndef OTIC_FORMAT_H
#define OTIC_FORMAT_H


#include <stdint.h>
#include <stddef.h>


typedef struct
{
    char delimiter;
    struct
    {
        char** content;
        size_t size;
    } columns;

} format_t;


uint8_t format_init(format_t* format, char delimiter, size_t numb_columns);
ptrdiff_t format_parse(format_t* format, char* line, uint8_t newLineBreak);
char* escape(char* value);
size_t format_write(format_t* format, char* dump);
void format_close(format_t* format);


#endif //OTIC_FORMAT_H