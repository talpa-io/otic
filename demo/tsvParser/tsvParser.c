//
// Created by talpaadmin on 23.10.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <format/format.h>
#include <assert.h>
#include <sys/stat.h>

#include "pack/pack.h"
#include "unpack/unpack.h"
#include "errHand/errHand.h"
#include "format/format.h"

// Almost 1 MB ... memory friendly 1 MB. Did you know that 1 byte is actually 9 bits and not 8 bits?
#define READ_BUFFERSIZE 1048576


static otic_errors_e otic_error;

typedef enum
{
    TSV_PARSER_ERROR_NONE,
    TSV_PARSER_ERROR_INVALID_INPUT,
    TSV_PARSER_ERROR_STRING_TOO_LONG,
    TSV_PARSER_ERROR_FILE,
    TSV_PARSER_ERROR_FORMATTER,
    TSV_PARSER_ERROR_OTIC
} tsv_parser_error_e;

static void toConsole(const char* message)
{
    fprintf(stderr, "%s", message);
}

static const char* tsv_parser_getError(tsv_parser_error_e error)
{
    switch (error)
    {
        case TSV_PARSER_ERROR_NONE:
            return "No error";
        case TSV_PARSER_ERROR_INVALID_INPUT:
            return "Invalid Input";
        case TSV_PARSER_ERROR_STRING_TOO_LONG:
            return "String too long";
        case TSV_PARSER_ERROR_FILE:
            return strerror(errno);
        case TSV_PARSER_ERROR_FORMATTER:
            return "Formatter Error";
        case TSV_PARSER_ERROR_OTIC:
            return otic_getError(otic_error);
        default:
            return "Unknown Error";
    }
}

static void toConsole_usage(void)
{
    toConsole("Usage: ");
    toConsole("./otic [-p|-u] [-i] inputFileName [-o] outputFileName\n");
}

inline static uint8_t same(const char* value1, const char* value2, uint32_t max)
{
    return strlen(value2) > max? 0 : strcmp(value1, value2) == 0;
}

static tsv_parser_error_e inputCheck(char** argv)
{
    if (!same("-p", argv[1], 2) && !same("-u", argv[1], 2))
        return TSV_PARSER_ERROR_INVALID_INPUT;
    if (!same("-i", argv[2], 2) || !same("-o", argv[4], 2))
        return TSV_PARSER_ERROR_INVALID_INPUT;
    if (strlen(argv[3]) > 24 || strlen(argv[5]) > 24)
        return TSV_PARSER_ERROR_STRING_TOO_LONG;
    return TSV_PARSER_ERROR_NONE;
}

inline static size_t power2(size_t value)
{
    size_t counter = 1;
    while (counter < value)
        counter <<= 1u;
    return counter;
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
    while (*value) {
        if (!isNumericChar(*value++))
            return 0;
    }
    return 1;
}

static FILE* inputFile = 0;
static FILE* outputFile = 0;

// TODO: CHANGE STREAM BUFFER
static inline uint8_t flusher(uint8_t* value, size_t size)
{
    fwrite(value, 1, size, outputFile);
    fflush(outputFile);
    return 1;
}

static inline uint8_t fetcher(uint8_t* value, size_t size)
{
    fread(value, 1, size, inputFile);
    return 1;
}

static inline uint8_t seeker(uint32_t pos)
{
    fseek(inputFile, pos, SEEK_CUR);
    return 1;
}

static inline uint8_t decPos(char* value, size_t length)
{
    char* ptr = value;
    while (*ptr && *ptr != '.')
        ptr++;
    return length - (ptr - value);
}

static inline uint8_t getDecPos(char* ptr)
{
    char* end;
    while (*ptr && *ptr != '.')
        ptr++;
    end = ptr;
    while(*end)
        end++;
    return end - ptr;
}

int fpeek(FILE* stream)
{
    int c = getc(stream);
    ungetc(c, stream);
    return c;
}

static inline uint8_t flusher2(uint8_t* buffer, size_t size)
{
    fwrite(buffer, 1, size, outputFile);
    return 1;
}

size_t getLinesNumber(const char* fileName)
{
    FILE* file = fopen(fileName, "r");
    size_t counter = 0;

    char buffer[512];
    while(!feof(file))
    {
        fscanf(file, "%[^\n]\n", buffer);
        ++counter;
    }
    return counter;
}

// TODO: 0.0000 == 0
static inline uint8_t compress(const char* fileNameIn, const char* fileNameOut)
{

    inputFile = fopen(fileNameIn, "r");
    outputFile = fopen(fileNameOut, "wb");
    if (!inputFile || !outputFile)
        return 0;

    otic_pack_t oticPack;
    if (!otic_pack_init(&oticPack, flusher))
        return 0;

    otic_pack_channel_t* channel = otic_pack_defineChannel(&oticPack, OTIC_CHANNEL_TYPE_SENSOR, 0x1, 0x0);
    if (!channel)
        return 0;

    format_chunker_t formatChunker;
    if (!format_init(&formatChunker.format, '\t', 5))
        return 0;

    char buffer[READ_BUFFERSIZE];
    size_t read = 0;
    while (fpeek(inputFile) != EOF)
    {
        read = fread(buffer, 1, READ_BUFFERSIZE - 1024, inputFile);
        read += fscanf(inputFile, "%[^\n]\n", buffer + read);
        buffer[read] = 0;
        format_chunker_set(&formatChunker, buffer, read);
        int64_t int_value = 0;
        uint8_t decPos = 0;
        while (formatChunker.ptr_current - formatChunker.ptr_start < formatChunker.size) {
            formatChunker.ptr_current = format_chunker_parse(&formatChunker);
            if (!formatChunker.format.columns.content[4])
            {
                 otic_pack_channel_inject_n(channel, strtod(formatChunker.format.columns.content[0], 0),
                                           formatChunker.format.columns.content[1],
                                           formatChunker.format.columns.content[3]);
            } else if (isNumeric(formatChunker.format.columns.content[4])) {
                if (!(decPos = getDecPos(formatChunker.format.columns.content[4])))
                {
                    if ((int_value = strtol(formatChunker.format.columns.content[4], 0, 10)) >= 0)
                    {
                        otic_pack_channel_inject_i(channel,
                                                   strtod(formatChunker.format.columns.content[0], 0),
                                                   formatChunker.format.columns.content[1],
                                                   formatChunker.format.columns.content[3],
                                                   int_value
                        );
                    } else {
                        otic_pack_channel_inject_i_neg(channel,
                                                       strtod(formatChunker.format.columns.content[0], 0),
                                                       formatChunker.format.columns.content[1],
                                                       formatChunker.format.columns.content[3],
                                                       -int_value
                        );
                    }
                } else {
                    otic_pack_channel_inject_d(channel,
                                               strtod(formatChunker.format.columns.content[0], 0),
                                               formatChunker.format.columns.content[1],
                                               formatChunker.format.columns.content[3],
                                               strtod(formatChunker.format.columns.content[4], 0)
                    );
                }
            } else {
                otic_pack_channel_inject_s(channel,
                                           strtod(formatChunker.format.columns.content[0], 0),
                                           formatChunker.format.columns.content[1],
                                           formatChunker.format.columns.content[3],
                                           formatChunker.format.columns.content[4]
                );
            }
        }
    }
    otic_pack_close(&oticPack);
    fclose(inputFile);
    fclose(outputFile);
    return 1;
}

uint8_t decompress(const char* fileNameIn, const char* fileNameOut)
{
    inputFile = fopen(fileNameIn, "rb");
    outputFile = fopen(fileNameOut, "w");
    if (!inputFile || !outputFile)
        return 0;
    otic_unpack_t oticUnpack;
    if (!otic_unpack_init(&oticUnpack, fetcher, seeker))
        return 0;
    if (!otic_unpack_defineChannel(&oticUnpack, 0x01, flusher2))
        return 0;
    size_t counter = 0;
    while(!feof(inputFile) && counter < 2)
    {
        otic_unpack_parse(&oticUnpack);
        printf("Counter: %lu\n", counter++);
    }

    otic_unpack_close(&oticUnpack);
    fclose(inputFile);
    fclose(outputFile);
    return 1;
}


// Total number of lines 5090023
// TODO: Add a otic_file info fetcher
// TODO: Decide Buffer Size: Either use BUFSIZE or 12000

int main(void)
{
  /*  struct stat stats;
    inputFile = fopen("dump.otic", "r");
    if (fstat(fileno(inputFile), &stats) == -1)
    {
        perror("fstat");
        return 1;
    }
    printf("Buffize: %d Optimal Size %ld\n", BUFSIZ, stats.st_blksize);*/

//    return !compress("bigFile.txt", "dump.otic");
    return !decompress("dump.otic", "output.tsv");

    return EXIT_SUCCESS;
}



/*
int main(int argc, char** argv)
{
    tsv_parser_error_e error;

    */
/*if (argc == 1) {
        toConsole_usage();
        return EXIT_FAILURE;
    }
    if (argc != 6) {
        toConsole("Invalid Input\n");
        toConsole_usage();
        return EXIT_FAILURE;
    }*//*


*/
/*    if ((error = inputCheck(argv) != TSV_PARSER_ERROR_NONE))
        goto fail;*//*


    size_t read = 0;

//    if (same("-p", argv[1], 2)) {
//        inputFile = fopen(argv[3], "r");
//        outputFile = fopen(argv[5], "wb");

        inputFile = fopen("bigFile.txt", "r");
        outputFile = fopen("dump.otic", "wb");
        if (!inputFile || !outputFile) {
            error = TSV_PARSER_ERROR_FILE;
            goto fail;
        }

        otic_pack_t oticPack;
        if (!otic_pack_init(&oticPack, flusher)) {
            otic_error = oticPack.error;
            goto fail;
        }
        otic_pack_channel_t* channel = otic_pack_defineChannel(&oticPack, OTIC_CHANNEL_TYPE_SENSOR, 1, 0x0);

        format_chunker_t formatChunker;
        char buffer[READ_BUFFERSIZE];
        if (!format_chunker_init(&formatChunker, '\t', 5)){
            error = TSV_PARSER_ERROR_FORMATTER;
            goto fail;
        }
        uint8_t decPos;
        tsv_content_t content;
        while (!feof(inputFile))
        {
            read = fread(buffer, 1, READ_BUFFERSIZE - 1024, inputFile);
            read += fscanf(inputFile, "%[^\n]\n", buffer + read);
            format_chunker_set(&formatChunker, buffer, read);
            uint32_t sCounter = 0;
            int32_t int_value;
            while (formatChunker.ptr_current - formatChunker.ptr_start < formatChunker.size)
            {
                formatChunker.ptr_current = format_chunker_parse(&formatChunker);
                char sBuffer[254] = {};
                format_write(&formatChunker.format, sBuffer);
                printf("%s\n", sBuffer);


                if (!formatChunker.format.columns.content[4])
                {
                    otic_pack_channel_inject_n(channel, strtod(formatChunker.format.columns.content[4], 0),
                                               formatChunker.format.columns.content[1],
                                               formatChunker.format.columns.content[3]);
                } else if (isNumeric(formatChunker.format.columns.content[4])) {
                    if (!(decPos = getDecPos(formatChunker.format.columns.content[4])))
                    {
                        if ((int_value = strtol(formatChunker.format.columns.content[4], 0, 10)) >= 0)
                        {
                            otic_pack_channel_inject_i(channel,
                                                       strtod(formatChunker.format.columns.content[0], 0),
                                                       formatChunker.format.columns.content[1],
                                                       formatChunker.format.columns.content[3],
                                                       int_value
                            );
                        } else {
                            otic_pack_channel_inject_i_neg(channel,
                                                        strtod(formatChunker.format.columns.content[0], 0),
                                                        formatChunker.format.columns.content[1],
                                                        formatChunker.format.columns.content[3],
                                                        -int_value
                                                        );
                        }
                    } else {
                        otic_pack_channel_inject_d(channel,
                                                   strtod(formatChunker.format.columns.content[0], 0),
                                                   formatChunker.format.columns.content[1],
                                                   formatChunker.format.columns.content[3],
                                                   strtod(formatChunker.format.columns.content[4], 0)
                                                    );
                    }
                } else {
                    otic_pack_channel_inject_s(channel,
                                               strtod(formatChunker.format.columns.content[0], 0),
                                               formatChunker.format.columns.content[1],
                                               formatChunker.format.columns.content[3],
                                               formatChunker.format.columns.content[4]
                            );
                }
            }
            break;
        }

        format_chunker_close(&formatChunker);
        otic_pack_close(&oticPack);
        fclose(inputFile);
        fclose(outputFile);

//    } else if (same("-u", argv[1], 2)) {
//        inputFile = fopen(argv[3], "rb");
//        outputFile = fopen(argv[5], "w");
//        if (!inputFile || !outputFile){
//            error = TSV_PARSER_ERROR_FILE;
//            goto fail;
//        }
//    } else {
//        error = TSV_PARSER_ERROR_INVALID_INPUT;
//        goto fail;
//    }
    return EXIT_SUCCESS;

fail:
    toConsole("Error: ");
    toConsole(tsv_parser_getError(error));
    toConsole("\n");
    toConsole_usage();
    return EXIT_FAILURE;
}*/