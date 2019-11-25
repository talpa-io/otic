#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <utility/format.h>
#include "utility/compare.h"
#include "utility/format.h"


// TODO: QUICK AND DIRTY COMPARISION FUNCTIONS. NEED TO BE CLEANED
#define COMPARE_CHUNK_SIZE 1048576

static inline int32_t fpeek(FILE* file)
{
    static int c;
    c = fgetc(file);
    ungetc(c, file);
    return c;
}

uint8_t compare_compareNumbLines(FILE* file1, FILE* file2)
{
    if (!file1 || !file2)
        return 0;
    char buffer[512];
    size_t counter1 = 0, counter2 = 0;
    while(fpeek(file1) != EOF)
    {
        (void)fscanf(file1, "%[^\n]\n", buffer);
        ++counter1;
    }
    while(fpeek(file2) != EOF)
    {
        (void)fscanf(file2, "%[^\n]\n", buffer);
        ++counter2;
    }
    printf("Res: %lu, %lu\n", counter1, counter2);
    return counter1 == counter2;
}

static inline char* cutVal(char* value, uint32_t needle)
{
    value[needle] = 0;
    return value;
}

static inline void printError(const char* compVal1, const char* compVal2, size_t line)
{
    fprintf(stderr, "Error: %s != %s. At line: %lu\n", compVal1, compVal2, line);
    exit(1);
}

inline static uint8_t isNumericChar(char value)
{
    static const char* numeric = "0123456789.-";
    const char* fCharacter = numeric;
    do {
        if (*fCharacter == value)
            return 1;
        ++fCharacter;
    } while (*fCharacter);
    return 0;
}

inline static uint8_t isNumeric(const char* value)
{
    while (*value)
        if (!isNumericChar(*value++))
            return 0;
    return 1;
}

uint8_t compare_compareLineValues(FILE* file1, FILE* file2)
{
    if (!file1 || !file2)
        return 0;
    format_t format1, format2;
    char buffer1[512], buffer2[512];
    if (!format_init(&format1, '\t', 5) || !format_init(&format2, '\t', 5))
        return 0;
    double val1, val2;
    size_t line = 0;
    while(fpeek(file1) != EOF)
    {
        (void)fscanf(file1, "%[^\n]\n", buffer1);
        (void)fscanf(file2, "%[^\n]\n", buffer2);
        format_parse(&format1, buffer1);
        format_parse(&format2, buffer2);

        for (uint8_t counter = 0; counter < 5; counter++)
        {
            switch (counter)
            {
                case 0:
                    if (strtod(cutVal(format1.columns.content[counter], 15), 0) != strtod(cutVal(format2.columns.content[counter], 15), 0)) {
                        printError(format1.columns.content[counter], format2.columns.content[counter], line);
                    }
                    break;
                case 1:
                    if (strcmp(format1.columns.content[counter], format2.columns.content[counter]) != 0) {
                        printError(format1.columns.content[counter], format2.columns.content[counter], line);
                    }
                    break;
                case 2:
                    if (format1.columns.content[counter + 1] == 0) {
                        if (format2.columns.content[counter] != 0)
                            printError("(null)", format2.columns.content[counter], line);
                    } else
                        if (strcmp(format1.columns.content[counter + 1], format2.columns.content[counter]) != 0)
                            printError(format1.columns.content[counter + 1], format2.columns.content[counter], line);
                    break;

                case 3:
                    if (format1.columns.content[counter + 1] == 0) {
                        if (format2.columns.content[counter] != 0)
                            printError("(null)", format2.columns.content[counter], line);
                    } else if (isNumeric(format1.columns.content[counter + 1])){
                        if (strtol(format1.columns.content[counter + 1], 0, 10)  != strtol(format2.columns.content[counter], 0, 10))
                            printError(format1.columns.content[counter + 1], format2.columns.content[counter], line);
                    }
                    else {
                        if (strcmp(format1.columns.content[counter + 1], format2.columns.content[counter]) != 0)
                            printError(format1.columns.content[counter + 1], format2.columns.content[counter], line);
                    }
                    break;
                default:
                    break;
            }
        }
        line++;
    }
    return 1;
}
