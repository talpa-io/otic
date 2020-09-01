#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fenv.h>

#include "core/pack.h"
#include "core/unpack.h"
#include "utility/errHand.h"
#include "utility/format.h"
#include "utility/compare.h"

#define READ_BUFFERSIZE 1048576

static otic_error_e otic_error;

typedef enum
{
    TSV_PARSER_ERROR_NONE,
    TSV_PARSER_ERROR_INVALID_INPUT,
    TSV_PARSER_ERROR_STRING_TOO_LONG,
    TSV_PARSER_ERROR_FILE,
    TSV_PARSER_ERROR_FORMATTER,
    TSV_PARSER_ERROR_OTIC
} tsv_parser_error_e;

static inline const char* tsv_parser_strError(tsv_parser_error_e error)
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
            return otic_strError(otic_error);
        default:
            return "Unknown Error";
    }
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

inline static size_t getMemFriendlyApprox(size_t value)
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
        if (*fCharacter++ == value)
            return 1;
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

// TODO: CHANGE STREAM BUFFER
static inline uint8_t flusher(uint8_t* value, size_t size, void* file)
{
    fwrite(value, 1, size, (FILE*)file);
    fflush((FILE*)file);
    return 1;
}

static inline uint8_t fetcher(uint8_t* value, size_t size, void* data)
{
    return fread(value, 1, size, (FILE*)data) != 0;
}

static inline uint8_t seeker(uint32_t pos, void* data)
{
    return fseek((FILE*)data, pos, SEEK_CUR) != -1;
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

static inline int fpeek(FILE* stream)
{
    int c = getc(stream);
    ungetc(c, stream);
    return c;
}

static inline uint8_t flusher2(double timestamp, const char* sensorName, const char* sensorUnit, const oval_t* val, void* data)
{
    switch (otic_oval_getType(val))
    {
        case OTIC_TYPE_INT_POS:
            fprintf((FILE*)data, "%lf\t%s\t%s\t%lu\n", timestamp, sensorName, sensorUnit, val->val.lval);
            return 1;
        case OTIC_TYPE_INT_NEG:
            fprintf((FILE*)data, "%lf\t%s\t%s\t-%lu\n", timestamp, sensorName, sensorUnit, val->val.lval);
            return 1;
        case OTIC_TYPE_DOUBLE:
            fprintf((FILE*)data, "%lf\t%s\t%s\t%lf\n", timestamp, sensorName, sensorUnit, val->val.dval);
            return 1;
        case OTIC_TYPE_STRING:
            fprintf((FILE*)data, "%lf\t%s\t%s\t%s\n", timestamp, sensorName, sensorUnit, val->val.sval.ptr);
            return 1;
        case OTIC_TYPE_NULL:
            fprintf((FILE*)data, "%lf\t%s\t%s\t\n", timestamp, sensorName, sensorUnit);
            return 1;
        default:
            return 0;
    }
}

inline static size_t getLinesNumber(const char* fileName)
{
    FILE* file = fopen(fileName, "r");
    size_t counter = 0;
    char buffer[512];
    int ret;
    while(!feof(file))
    {
        ret = fscanf(file, "%[^\n]\n", buffer);
        ++counter;
    }
    return counter;
}

// TODO: 0.0000 == 0
/**
 * @param fileNameIn
 * @param fileNameOut
 * @return Otic_error number. 0 for success, 1 in case of failure.
 * Use \a otic_getError to print the error.
 */
static inline uint8_t compress(const char* fileNameIn, const char* fileNameOut) {
    tsv_parser_error_e tsvParserError = TSV_PARSER_ERROR_NONE;
    otic_error = OTIC_ERROR_NONE;
    FILE *inputFile = fopen(fileNameIn, "r");
    FILE *outputFile = fopen(fileNameOut, "wb");
    if (!inputFile || !outputFile) {
        tsvParserError = TSV_PARSER_ERROR_FILE;
        goto fail;
    }

    otic_pack_t oticPack;
    if (!otic_pack_init(&oticPack, 0x00, flusher, outputFile)) {
        tsvParserError = TSV_PARSER_ERROR_OTIC;
        goto fail;
    }

    otic_pack_channel_t* channel = otic_pack_defineChannel(&oticPack, OTIC_CHANNEL_TYPE_SENSOR, 0x1, 0x00, 0);
    if (!channel) {
        tsvParserError = TSV_PARSER_ERROR_OTIC;
        goto fail;
    }
    format_chunker_t formatChunker;
    if (!format_init(&formatChunker.format, '\t', 5))
    {
        tsvParserError = TSV_PARSER_ERROR_FORMATTER;
        goto fail;
    }
    char buffer[READ_BUFFERSIZE];
    size_t read = 0;
    char* end = 0;
    int scanned = 0;
    while (fpeek(inputFile) != EOF) {
        read = fread(buffer, 1, READ_BUFFERSIZE - 1024, inputFile);
        buffer[read] = 0;
        scanned = fscanf(inputFile, "%[^\n]\n", (end = buffer + read));
        if (scanned != -1)
            read += strlen(end);
        if (scanned == 0)
        {
            int got;
            if ((got = fgetc(inputFile)) != '\n')
                ungetc(got, inputFile);
        }
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

            if (oticPack.state == OTIC_STATE_ON_ERROR)
                goto fail;
        }
    }
    otic_pack_close(&oticPack);
    format_chunker_close(&formatChunker);
    fclose(inputFile);
    fclose(outputFile);
    // TODO: Add statistics
    return otic_error;
fail:
    if (tsvParserError == TSV_PARSER_ERROR_OTIC)
        otic_error = oticPack.error;
    fprintf(stderr, "%s\n", tsv_parser_strError(tsvParserError));
    return 0;
}

inline static uint8_t decompress(const char* fileNameIn, const char* fileNameOut)
{
    FILE* inputFile = fopen(fileNameIn, "rb");
    FILE* outputFile = fopen(fileNameOut, "w");
    uint8_t error;
    if (!inputFile || !outputFile)
        return 0;

    otic_unpack_t oticUnpack;
    if (!otic_unpack_init(&oticUnpack, fetcher, inputFile, seeker, inputFile))
        goto fail;
    oticUnpackChannel_t* channel;
    if (!(channel = otic_unpack_defineChannel(&oticUnpack, 0x01, flusher2, outputFile)))
        goto fail;
    otic_unpack_channel_toFetch(channel, 0, 0);
//    while(fpeek(inputFile) != EOF)
//        otic_unpack_parse(&oticUnpack);
    while(otic_unpack_parse(&oticUnpack));
    if (oticUnpack.state == OTIC_STATE_ON_ERROR)
        printOticError(oticUnpack.error);
    error = oticUnpack.channels[0].base.error;
    otic_unpack_close(&oticUnpack);
    fclose(inputFile);
    fclose(outputFile);
    return error;

fail:
    printOticError(oticUnpack.error);
    return 0;
}

// TODO: Add a otic_file info fetcher
// TODO: Decide Buffer Size: Either use BUFSIZE or 12000 (Mem. friendly = 16384)
static uint8_t compare(const char* origFileName, const char* decompFileName)
{
    FILE* originalFile = fopen(origFileName, "r");
    FILE* decompFile = fopen(decompFileName, "r");
    if (!originalFile || !decompFile)
        return 0;
    return compare_compareLineValues(originalFile, decompFile);
}

static uint8_t getLines(const char* fileInName, const char* fileOutName, size_t size)
{
    FILE* inputFile = fopen(fileInName, "r");
    FILE* outputFile = fopen(fileOutName, "w");
    char buffer[254];
    size_t counter;
    int ret;
    for (counter = 0; counter < size; counter++)
    {
        ret = fscanf(inputFile, "%[^\n]\n", buffer);
        fwrite(buffer, 1, strlen(buffer), outputFile);
        fwrite("\n", 1, 1, outputFile);
    }
    return counter;
}

#ifdef DEBUG
int main(void)
{
    const char* argv[] = {"otic", "-u", "-i", "dump.otic", "-o", "dump.tsv"};
//    const char* argv[] = {"otic", "-u", "-i", "dump.otic", "-o", "res.txt"};
    int argc = 6;
#else
int main(int argc, char** argv)
{
#endif

    if (argc == 1)
        goto usage;
    if (argc == 2 && same("-h", argv[1], 2))
        goto usage;
    if (argc == 2 && same("-v", argv[1], 2))
        goto version;
    if (argc == 6)
    {
        fesetround(FE_UPWARD);
        if (same("-i", argv[2], 2) && same("-o", argv[4], 2)) {
            if (same("-p", argv[1], 2))
                return compress(argv[3], argv[5]);
            else if (same("-u", argv[1], 2))
                return decompress(argv[3], argv[5]);
            else if (same("-c", argv[1], 2))
                return compare(argv[3], argv[5]);
        }
    }

// Fallthrough

invalid_input:
    fprintf(stderr, "%s", "Invalid Input\n");
usage:
    fprintf(stderr, "%s", "Usage: otic [-p|-u|-c|-h|-v] [-i] <inputFileName> [-o] <outputFileName>\n");
    return EXIT_FAILURE;
version:
    fprintf(stderr, "v%d.%d.%d\n", OTIC_VERSION_MAJOR, OTIC_VERSION_MINOR, OTIC_VERSION_PATCH);
    return EXIT_SUCCESS;
}
