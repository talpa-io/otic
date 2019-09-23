#include "stdio.h"
#include <zstd.h>


// BUFSIZE must fit one value write, which in the worst case includes
// - the file header (HEADER_SIZE)
// - starting a block (BLOCK_HEADER_SIZE)
// - registering a column with metadata (2 * 64k + a bit)
// - writing a 64k string value
// so let's use 4*64k = 256k to be on the save side
//
// however, the BUFSIZE is only checked after every value written, so we make
// the buffer 2x larger and check whether we are above the halfway point of the
// buffer

#define BUFSIZE (4*64*1024)

#define FILEFORMAT_VERSION 1

#define STRING_SIZE_LIMIT 65536

// message types
#define MAX_DIRECT_INT 200

#define TYPE_NULL 201
#define TYPE_EMPTY_STRING 204

#define TYPE_POS_INT 210
#define TYPE_NEG_INT 211

#define TYPE_DOUBLE 220
#define TYPE_FLOAT 221

#define TYPE_STRING 230
#define TYPE_UNMODIFIED 231

#define TYPE_SHIFT_TIMESTAMP_EPOCH 240
#define TYPE_SHIFT_TIMESTAMP_NS 241

#define TYPE_NAME_ASSIGN 251
#define TYPE_META_ASSIGN 252


// block headers

#define BLOCK_TYPE_DATA 0
#define BLOCK_TYPE_RAW 1
#define BLOCK_TYPE_END 2

#define BLOCK_HEADER_SIZE 13
#define HEADER_SIZE 7


// feature flags
#define FEATURE_COMPRESSION 1

// tagged union types
#define OTIC_TYPE_NONE_SEEN 0


int encode_varint_unsigned(unsigned long value, char* target);

struct otic_column {
    otic_writer writer;
    unsigned long columnindex;

    union {
        long int_value;
        double double_value;
        size_t string_size;
    } last_value;
    int last_value_type;

    char * last_string_buffer; // NULL to start with

    char * name; // NULL to start with
};

struct otic_writer {
    otic_column* columnarray; // array of columns
    size_t numcolumns;
    size_t capcolumns; // capacity of overallocated array

    // either outfile is not NULL, or write_callback and userdata are set
    FILE* outfile;

    // compressor is NULL is compression is disabled
    ZSTD_CCtx* compressor;

    otic_write_cb_tp write_callback;
    void* userdata;


    time_t first_epoch, last_epoch;
    long first_nanoseconds, last_nanoseconds;

    bool seen_timestamp_in_block;

    bool header_written;
    bool block_started;

    // block_header is either a prefix of buffer, or of compressed_buffer
    char* block_header;
    char* buffer;
    size_t position;

    char* compressed_buffer;
    char* other_to_free;
};

otic_writer _writer_open(void);
otic_writer _create_compressor(otic_writer result);
void _writer_dealloc(otic_writer);
void _column_dealloc(otic_column);
void _write_varuint(otic_writer, unsigned long);
void _write_byte(otic_writer, uint8_t);
int _write_string_size(otic_writer w, char* value, size_t size);

int _maybe_flush(otic_writer);
int _maybe_write_column(otic_column);
void _write_header(otic_writer);
int _write_column_id_assignment(otic_writer, char*, size_t);

int _write_to_output(otic_writer w, char* buffer, size_t length);
void _ensure_block_started(otic_writer w);
void _end_block(otic_writer w, size_t size);
void _write_uint32(char* buffer, uint32_t size);

otic_column _get_column(otic_writer w, size_t index);


struct otic_reader {
    otic_result* columnarray; // array of column, by index
    size_t numcolumns;
    size_t capcolumns; // capacity of overallocated array

    FILE* infile;
    bool owns_infile;

    int errorcode;

    // compressor is NULL is compression is disabled
    ZSTD_DCtx* decompressor;

    char* buffer;
    char* uncompressed_buffer;
    size_t position;
    size_t block_size;

    int version;

    time_t last_epoch;
    long last_nanoseconds;
};

struct otic_result {
    unsigned long columnindex;
    otic_reader reader;

    union {
        long int_value;
        double double_value;
        size_t string_size;
    } last_value;
    int last_value_type;

    char * last_string_buffer; // NULL to start with, needs to be freed

    char * name;
    size_t lenname;
    char * metadata;
    size_t lenmetadata;

    bool ignore_column;
};


void _reader_dealloc(otic_reader);
void _result_dealloc(otic_result res);
int _ensure_header_read(otic_reader);
int _read_block(otic_reader);

otic_result _otic_reader_next_every_column(otic_reader r);

uint32_t _read_uint32(uint8_t*);
int _read_byte(otic_reader r, uint8_t* result);
int _read_string(otic_reader r, char** result, size_t* length);
int _read_string_copy(otic_reader r, char** result, size_t* length);
int _read_varuint(otic_reader r, unsigned long* result);
int _read_varuint_slow_path(unsigned long res, otic_reader r, unsigned long* result);
int _add_new_column(otic_reader r);


otic_result _return_value_long(otic_result column, long value);
otic_result _return_value_double(otic_result column, double value);
otic_result _return_value_string(otic_result column, char* value, size_t size);
otic_result _return_value_null(otic_result column);
