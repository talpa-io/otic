#ifndef OTIC_COMPARE_H
#define OTIC_COMPARE_H
#include <stdint.h>

uint8_t compare_compareNumbLines(FILE* file1, FILE* file2);
uint8_t compare_compareLineValues(FILE *file1, FILE *file2);

#endif // OTIC_COMPARE